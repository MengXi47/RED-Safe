"""UI 應用的主要視圖與 API。"""

import asyncio
import base64
import io
import json
import logging
import re
import secrets
import subprocess
import threading
from collections.abc import Coroutine
from ipaddress import IPv4Address, IPv6Address, IPv4Network, ip_address, ip_network
from typing import Any, Dict, Optional
from urllib.parse import quote
def _current_ipv4_network() -> IPv4Network | None:
    """取出預設介面的 IPv4 網段（如 192.168.1.0/24）。失敗回傳 None。"""
    try:
        gws = netifaces.gateways()
        if 'default' not in gws or netifaces.AF_INET not in gws['default']:
            return None
        iface = gws['default'][netifaces.AF_INET][1]
        addrs = netifaces.ifaddresses(iface)
        ip_info = addrs.get(netifaces.AF_INET, [{}])[0]
        addr = ip_info.get('addr')
        netmask = ip_info.get('netmask')
        if not addr or not netmask:
            return None
        # 允許 non-strict，避免 netmask 與位址不完全吻合時拋例外
        return ip_network(f"{addr}/{netmask}", strict=False)  # type: ignore[return-value]
    except Exception:
        return None

import netifaces
import psutil
import qrcode
import requests
from django.conf import settings
from django.http import HttpRequest, HttpResponse, JsonResponse
from django.middleware.csrf import get_token
from django.shortcuts import redirect, render
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_http_methods,require_POST
from ipcscan_client.ipcsacn import scan_ipc_dynamic

from cloudclient.httpclient import get_bound_users, remove_bound_user
from .models import ConnectedIPC, EdgeConfig

try:
    from aiortc import RTCPeerConnection, RTCSessionDescription
    from aiortc.contrib.media import MediaPlayer, MediaRelay

    AIORTC_AVAILABLE = True
except ImportError:  # pragma: no cover - optional dependency
    RTCPeerConnection = None  # type: ignore[assignment]
    RTCSessionDescription = None  # type: ignore[assignment]
    MediaPlayer = None  # type: ignore[assignment]
    MediaRelay = None  # type: ignore[assignment]
    AIORTC_AVAILABLE = False


def show_users(request):
    """顯示 config 表中的所有資料列供前端檢視。"""

    rows = EdgeConfig.objects.all()
    return render(request, "ui/users.html", {"rows": rows})


SESSION_KEY = "setup_authed"

# 密碼規則：僅英數、至少 6 碼
PASSWORD_REGEX = re.compile(r'^[A-Za-z0-9]{6,}$')

DEFAULT_EDGE_VERSION = getattr(settings, "EDGE_VERSION", "1.0.0")

logger = logging.getLogger(__name__)


def _get_edge_config() -> EdgeConfig | None:
    """取得唯一一筆 edge 設定資料。"""

    return EdgeConfig.objects.first()


def _edge_id() -> str:
    """從 config 表讀出 edge_id，若不存在則回傳預設值。"""

    config = _get_edge_config()
    return config.edge_id if config else "RED-UNKNOWN"


def _is_configured() -> bool:
    """檢查 config 表中的密碼是否已設定完成。"""

    config = _get_edge_config()
    return bool(config and config.edge_password)


def _require_auth(view_func):
    """登入保護裝飾器：未設定導向 setup，未登入導向 login。"""

    def wrapper(request: HttpRequest, *args, **kwargs):
        if not _is_configured():
            return redirect("setup")
        if not request.session.get(SESSION_KEY):
            return redirect("login")
        return view_func(request, *args, **kwargs)

    return wrapper


def _pi_temperature() -> str:
    """讀取樹莓派 CPU 溫度；失敗時回傳 N/A。"""

    try:
        out = subprocess.check_output(["vcgencmd", "measure_temp"]).decode()
        return out.replace("temp=", "").replace("'C", "°C").strip()
    except Exception:
        try:
            with open("/sys/class/thermal/thermal_zone0/temp") as f:
                return f"{int(f.read()) / 1000:.1f}°C"
        except Exception:
            return "N/A"


@require_http_methods(["GET", "POST"])
def setup_view(request: HttpRequest):
    """首次設定裝置密碼並向遠端註冊裝置。"""

    if _is_configured():
        return redirect("login")

    ctx = {"csrf_token": get_token(request), "error": None}
    config = _get_edge_config()
    if not config:
        ctx["error"] = "資料庫缺少 config 設定，請聯絡管理員。"
        return render(request, "ui/setup.html", ctx)

    if request.method == "POST":
        pwd = (request.POST.get("password") or "").strip()
        confirm = (request.POST.get("confirm") or "").strip()

        if not PASSWORD_REGEX.fullmatch(pwd):
            ctx["error"] = "密碼需至少 6 碼，且僅能使用英文或數字（不可空白、中文或符號）。"
            return render(request, "ui/setup.html", ctx)

        if pwd != confirm:
            ctx["error"] = "兩次輸入不一致。"
            return render(request, "ui/setup.html", ctx)

        config.edge_password = pwd
        config.save(update_fields=["edge_password"])

        base = getattr(settings, "REMOTE_API_BASE", "https://api.redsafe-tw.com").rstrip("/")
        url = base + "/edge/reg"
        timeout = getattr(settings, "REMOTE_API_TIMEOUT", 5)
        payload = {
            "edge_id": config.edge_id,
            "edge_password": pwd,
            "version": DEFAULT_EDGE_VERSION,
        }
        try:
            resp = requests.post(url, json=payload, timeout=timeout)
            data = {}
            try:
                data = resp.json()
            except Exception:
                pass
            if resp.status_code == 200 and data.get("edge_id") == config.edge_id:
                request.session[SESSION_KEY] = True
                return redirect("dashboard")
            ctx["error"] = f"遠端註冊失敗：HTTP {resp.status_code} 回應 {data}"
        except requests.RequestException as exc:
            ctx["error"] = f"遠端註冊連線失敗：{exc}"

    return render(request, "ui/setup.html", ctx)


@require_http_methods(["GET", "POST"])
def login_view(request: HttpRequest):
    """驗證輸入密碼是否與 config 表一致後登入。"""

    ctx = {"csrf_token": get_token(request), "error": None}
    config = _get_edge_config()
    if not config:
        ctx["error"] = "資料庫缺少 config 設定，請聯絡管理員。"
        return render(request, "ui/login.html", ctx)

    if request.method == "POST":
        pwd = (request.POST.get("password") or "").strip()

        if not PASSWORD_REGEX.fullmatch(pwd):
            ctx["error"] = "密碼格式不正確：僅能使用英文或數字，且至少 6 碼。"
            return render(request, "ui/login.html", ctx)

        if pwd == (config.edge_password or ""):
            request.session[SESSION_KEY] = True
            return redirect("dashboard")

        ctx["error"] = "密碼錯誤。"

    return render(request, "ui/login.html", ctx)


def logout_view(request: HttpRequest):
    """登出並回到登入頁面。"""

    request.session.pop(SESSION_KEY, None)
    return redirect("login")


# ====== 受保護頁面 ======
@_require_auth
def dashboard(request: HttpRequest):
    """顯示儀表板頁面。"""

    return render(request, "ui/dashboard.html")


@_require_auth
def devices(request: HttpRequest):
    """顯示裝置資訊頁面。"""

    return render(request, "ui/devices.html")


@_require_auth
def logs(request: HttpRequest):
    """顯示日誌頁面。"""

    return render(request, "ui/logs.html")


@_require_auth
def settings_view(request: HttpRequest):
    """顯示設定頁面。"""

    return render(request, "ui/settings.html")


def healthz(request: HttpRequest):
    """供健康檢查使用的簡單回應。"""

    return HttpResponse("ok")


# ====== 資料庫概覽頁（含表頭） ======
@_require_auth
def db_overview(request: HttpRequest):
    """展示 config 表中的資料內容。"""

    rows = EdgeConfig.objects.all()
    return render(request, "ui/db_overview.html", {"rows": rows})


# --- Network ---
@_require_auth
def network_ip(request):
    """顯示目前的網路介面與 IP 資訊。"""

    ip_address = "N/A"
    netmask = "N/A"
    gateway = "N/A"
    dns = []

    try:
        # 取預設網路介面（通常是 eth0 或 wlan0）
        gws = netifaces.gateways()
        default_iface = (
            gws['default'][netifaces.AF_INET][1]
            if 'default' in gws and netifaces.AF_INET in gws['default']
            else None
        )

        if default_iface:
            addrs = netifaces.ifaddresses(default_iface)
            ip_info = addrs.get(netifaces.AF_INET, [{}])[0]
            ip_address = ip_info.get('addr', "N/A")
            netmask = ip_info.get('netmask', "N/A")
            gateway = gws['default'][netifaces.AF_INET][0]

        # 取 DNS
        with open("/etc/resolv.conf") as f:
            for line in f:
                if line.startswith("nameserver"):
                    dns.append(line.split()[1])
    except Exception as e:
        print("Network info error:", e)

    context = {
        "ip_address": ip_address,
        "netmask": netmask,
        "gateway": gateway,
        "dns": dns,
    }
    return render(request, "ui/network_ip.html", context)


@_require_auth
@require_http_methods(["GET"])
def api_user_bound(request: HttpRequest):
    """回傳綁定使用者清單，資料來源為雲端 edge API。"""

    edge_id = _edge_id()
    if not edge_id or edge_id == "RED-UNKNOWN":
        return JsonResponse(
            {"items": [], "count": 0, "error": "edge_id_not_configured"},
            status=400,
        )

    api_result = get_bound_users(edge_id)
    if not isinstance(api_result, dict):
        return JsonResponse(
            {"items": [], "count": 0, "error": "unexpected_response"},
            status=502,
        )

    if "error" in api_result:
        status_code = api_result.get("status")
        if not isinstance(status_code, int) or status_code < 400:
            status_code = 502
        detail = {k: v for k, v in api_result.items() if k != "error"}
        return JsonResponse(
            {"items": [], "count": 0, "error": api_result["error"], "detail": detail},
            status=status_code,
        )

    users = api_result.get("users") or []
    items = [
        {
            "email": user.get("email", ""),
            "user_name": user.get("user_name", ""),
            "bind_at": user.get("bind_at", ""),
            "last_online": user.get("last_online", ""),
        }
        for user in users
    ]

    return JsonResponse({"items": items, "count": len(items)})


@_require_auth
@require_http_methods(["GET"])
def api_network_ip(request: HttpRequest):
    """提供 IP 頁面輪詢用的即時網路資訊。"""
    ip_address = "N/A"
    netmask = "N/A"
    gateway = "N/A"
    dns = []
    iface = None
    try:
        gws = netifaces.gateways()
        if 'default' in gws and netifaces.AF_INET in gws['default']:
            gateway = gws['default'][netifaces.AF_INET][0]
            iface = gws['default'][netifaces.AF_INET][1]
        if iface:
            addrs = netifaces.ifaddresses(iface)
            ip_info = addrs.get(netifaces.AF_INET, [{}])[0]
            ip_address = ip_info.get('addr', "N/A")
            netmask = ip_info.get('netmask', "N/A")
        # DNS
        with open("/etc/resolv.conf") as f:
            for line in f:
                if line.startswith("nameserver"):
                    dns.append(line.split()[1])
    except Exception as e:
        print("Network info API error:", e)

    return JsonResponse({
        "iface": iface or "N/A",
        "ip_address": ip_address,
        "netmask": netmask,
        "gateway": gateway,
        "dns": dns,
    })


@csrf_exempt
@_require_auth
@require_http_methods(["POST"])
def api_user_remove(request: HttpRequest):
    """解除綁定指定使用者。"""

    edge_id = _edge_id()
    if not edge_id or edge_id == "RED-UNKNOWN":
        return JsonResponse(
            {"status": "error", "message": "edge_id_not_configured"},
            status=400,
        )

    try:
        payload = json.loads(request.body or "{}")
    except json.JSONDecodeError:
        return JsonResponse(
            {"status": "error", "message": "invalid_json"},
            status=400,
        )

    email = str(payload.get("email", "")).strip()
    if not email:
        return JsonResponse(
            {"status": "error", "message": "email_required"},
            status=400,
        )

    api_result = remove_bound_user(edge_id, email)
    if not isinstance(api_result, dict):
        return JsonResponse(
            {"status": "error", "message": "unexpected_response"},
            status=502,
        )

    if "error" in api_result:
        status_code = api_result.get("status")
        if not isinstance(status_code, int) or status_code < 400:
            status_code = 502
        detail = {k: v for k, v in api_result.items() if k != "error"}
        return JsonResponse(
            {"status": "error", "message": api_result["error"], "detail": detail},
            status=status_code,
        )

    error_code = str(api_result.get("error_code", ""))
    if error_code != "0":
        return JsonResponse(
            {"status": "error", "message": error_code or "unknown_error"},
            status=400,
        )

    return JsonResponse({"status": "ok"})


@_require_auth
def network_port(request):
    """顯示埠號資訊頁面，預留未來擴充。"""

    return render(request, "ui/network_port.html", {"port": 8000})


# --- 裝置 ---


@_require_auth
@require_http_methods(["GET", "POST"])
def device_change_password(request):
    """變更裝置密碼並同步更新遠端伺服器。"""

    errors: dict[str, str] = {}
    config = _get_edge_config()

    if not config:
        errors["old_password"] = "資料庫缺少 config 設定，請聯絡管理員。"
    elif request.method == "POST":
        old = (request.POST.get("old_password") or "").strip()
        new1 = (request.POST.get("new_password1") or "").strip()
        new2 = (request.POST.get("new_password2") or "").strip()

        if old != (config.edge_password or ""):
            errors["old_password"] = "舊密碼錯誤"

        if not new1:
            errors["new_password1"] = "新密碼不可為空"
        elif len(new1) < 6:
            errors["new_password1"] = "新密碼至少 6 碼"
        elif not new1.isalnum():
            errors["new_password1"] = "僅限英數（不可空白、符號、中文）"

        if not new2:
            errors["new_password2"] = "請再次輸入新密碼"
        elif new1 and new1 != new2:
            errors["new_password2"] = "兩次輸入的新密碼不一致"

        if not errors:
            base = getattr(settings, "REMOTE_API_BASE", "https://api.redsafe-tw.com").rstrip("/")
            url = base + "/edge/update/edge_password"
            timeout = getattr(settings, "REMOTE_API_TIMEOUT", 5)
            payload = {
                "edge_id": config.edge_id,
                "edge_password": old,
                "new_edge_password": new1,
            }
            try:
                resp = requests.post(url, json=payload, timeout=timeout)
                data = {}
                try:
                    data = resp.json()
                except Exception:
                    pass
                if resp.status_code == 200 and str(data.get("error_code")) == "0":
                    config.edge_password = new1
                    config.save(update_fields=["edge_password"])
                    request.session.flush()
                    return redirect("login")
                errors["new_password2"] = (
                    f"遠端更新失敗：HTTP {resp.status_code} code={data.get('error_code')}"
                )
            except requests.RequestException as exc:
                errors["new_password2"] = f"遠端連線失敗：{exc}"

    return render(request, "ui/device_change_password.html", {"errors": errors})


@_require_auth
@require_http_methods(["GET"])
def api_metrics(request: HttpRequest):
    """提供儀表板輪詢用的即時系統資源資訊。"""
    cpu_percent = psutil.cpu_percent(interval=0.3)

    mem = psutil.virtual_memory()
    ram_used_mb = round(mem.used / (1024 ** 2))
    ram_total_mb = round(mem.total / (1024 ** 2))
    ram_percent = mem.percent

    disk = psutil.disk_usage("/")
    disk_used_gb = round(disk.used / (1024 ** 3), 1)
    disk_total_gb = round(disk.total / (1024 ** 3), 1)
    disk_percent = disk.percent

    return JsonResponse({
        "cpu": {"percent": cpu_percent},
        "ram": {"used_mb": ram_used_mb, "total_mb": ram_total_mb, "percent": ram_percent},
        "disk": {"used_gb": disk_used_gb, "total_gb": disk_total_gb, "percent": disk_percent},
        "temperature": _pi_temperature(),
    })


@_require_auth
@require_http_methods(["GET"])
def api_network_port(request: HttpRequest):
    """回傳 HTTP/HTTPS/SSH 的監聽狀態與埠號，用於前端每 3 秒輪詢。"""
    host = (request.get_host().split(":")[0] or "127.0.0.1")

    listening = set()
    try:
        for c in psutil.net_connections(kind="inet"):
            if c.status == psutil.CONN_LISTEN and c.laddr:
                listening.add(int(c.laddr.port))
    except Exception as e:
        print("net_connections error:", e)

    def group(ports):
        found = sorted(p for p in listening if p in ports)
        return {"listening": bool(found), "ports": found}

    data = {
        "host": host,
        "http": group({80, 8000, 8080, 8888}),  # 常見 HTTP 埠
        "https": group({443, 8443}),  # 常見 HTTPS 埠
        "ssh": group({22}),  # SSH
    }
    return JsonResponse(data)


@_require_auth
def user_bound(request):
    """顯示綁定使用者頁面，目前以空清單呈現。"""

    return render(request, "ui/user_bound.html", {"users": []})


# 替換現有 device_info view 為下面版本
@_require_auth
def device_info(request):
    """顯示裝置基本資訊與預設 QR Code。"""

    config = _get_edge_config()
    serial = config.edge_id if config and config.edge_id else "RED-UNKNOWN"
    password = (config.edge_password or "") if config else ""

    raw_version = str(DEFAULT_EDGE_VERSION or "1.0.0")
    version = raw_version if raw_version.lower().startswith("v") else f"v{raw_version}"
    status = 1  # 1 = 已連線, 0 = 未連線
    name = "裝置"
    has_password = bool(password)
    masked_password = "＊" * len(password) if has_password else ""

    # QR code 內容格式：JSON，包含 serial / password / name 三個欄位
    payload_dict = {"serial": serial, "password": password, "name": name}
    payload = json.dumps(payload_dict, ensure_ascii=False, separators=(",", ":"))

    # 產生 QR image 至 memory buffer，轉成 base64 data URI
    try:
        qr = qrcode.QRCode(
            version=1,
            error_correction=qrcode.constants.ERROR_CORRECT_M,
            box_size=8,
            border=2,
        )
        qr.add_data(payload)
        qr.make(fit=True)
        img = qr.make_image(fill_color="black", back_color="white")

        buf = io.BytesIO()
        img.save(buf, format="PNG")
        buf.seek(0)
        b64 = base64.b64encode(buf.read()).decode("ascii")
        qrcode_data = f"data:image/png;base64,{b64}"
    except Exception as e:
        # 如果 QR 生成失敗，避免整個頁面掛掉，回傳 None
        qrcode_data = None
        print("QR generation error:", e)

    info = {
        "serial": serial,
        "version": version,
        "status": status,
        "password": password,
        "name": name,
        "masked_password": masked_password,
        "has_password": has_password,
    }
    return render(request, "ui/device_info.html", {"info": info, "qrcode_data": qrcode_data})


# ====== QR Image Endpoint ======
@_require_auth
@require_http_methods(["GET"])
def device_qr(request: HttpRequest):
    """產生裝置資訊（JSON 格式）的 QR Code 影像。"""

    serial = "12345678"
    password = "12345678"
    name="裝置"
    payload_dict = {"serial": serial, "password": password, "name": name}
    payload = json.dumps(payload_dict, ensure_ascii=False, separators=(",", ":"))

    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.constants.ERROR_CORRECT_M,
        box_size=8,
        border=2,
    )
    qr.add_data(payload)
    qr.make(fit=True)
    img = qr.make_image(fill_color="black", back_color="white")

    buf = io.BytesIO()
    img.save(buf, format="PNG")
    buf.seek(0)
    return HttpResponse(buf.getvalue(), content_type="image/png")

# ====== camera =====
def _ip_sort_key(value):
    """將 IP 轉為排序 key，IPv4 會排在 IPv6 之前。"""
    if not value:
        return (2, 0)
    try:
        addr = ip_address(value)
        family_rank = 0 if isinstance(addr, IPv4Address) else 1
        return (family_rank, int(addr))
    except ValueError:
        return (2, 0)


def _normalize_search_rows(raw_rows):
    """整理 IPC 掃描結果，統一欄位格式並去除重複。"""
    normalized = []
    seen_pairs = set()
    ip_candidate_keys = (
        "ip",
        "ip_address",
        "ipv4",
        "ipv4_address",
        "ipAddr",
        "ipaddr",
        "address",
    )
    mac_candidate_keys = (
        "mac",
        "mac_address",
        "macAddr",
        "macaddr",
        "macAddress",
    )
    name_candidate_keys = (
        "name",
        "ipc_name",
        "custom_name",
        "device_name",
        "model",
        "label",
    )

    for item in raw_rows or []:
        if not isinstance(item, dict):
            continue

        ip_value = ""
        for key in ip_candidate_keys:
            value = item.get(key)
            if value is None:
                continue
            value = str(value).strip()
            if value:
                ip_value = value
                break

        mac_raw_value = ""
        for key in mac_candidate_keys:
            value = item.get(key)
            if value is None:
                continue
            value = str(value).strip()
            if value:
                mac_raw_value = value
                break
        mac_value = mac_raw_value.replace("-", ":").upper()

        name_value = ""
        for key in name_candidate_keys:
            value = item.get(key)
            if value is None:
                continue
            value = str(value).strip()
            if value:
                name_value = value
                break

        if not (ip_value or mac_value):
            continue
        key = (ip_value, mac_value)
        if key in seen_pairs:
            continue
        seen_pairs.add(key)
        normalized.append({
            "ip": ip_value,
            "mac": mac_value,
            "name": (name_value or "IPC"),
        })
    return normalized


def _load_bound_entries():
    """讀取資料庫中的已綁定攝影機資訊並建立查找表。"""
    bound = []
    bound_map_by_ip = {}
    bound_map_by_mac = {}
    try:
        rows = ConnectedIPC.objects.all()
        for r in rows:
            ip_value = (r.ip_address or "").strip()
            mac_value = (r.mac_address or "").replace("-", ":").strip().upper()
            custom_name_value = (r.custom_name or "").strip()
            ipc_name_value = (r.ipc_name or "").strip()
            name_value = custom_name_value or ipc_name_value or mac_value or "IPC"
            entry = {
                "ip": ip_value,
                "mac": mac_value,
                "name": name_value,
            }
            bound.append(entry)
            if ip_value:
                bound_map_by_ip[ip_value] = entry
            if mac_value:
                bound_map_by_mac[mac_value] = entry
    except Exception as exc:  # noqa: BLE001
        print("ConnectedIPC query error:", exc)
        bound = []
        bound_map_by_ip = {}
        bound_map_by_mac = {}

    bound.sort(key=lambda item: (_ip_sort_key(item["ip"]), item["mac"]))
    return bound, bound_map_by_ip, bound_map_by_mac


def _merge_scan_with_bound(bound, bound_map_by_ip, bound_map_by_mac, raw_search_results):
    """將掃描結果與已綁定清單合併：
    1) 上方搜尋清單要顯示『已綁定』項，但只限於 **目前區網** 的已綁定。
    2) 對於掃描到的裝置，一律顯示（因為即屬目前區網），並標記 is_bound。
    3) 若有同區網的已綁定裝置沒被掃描到，也補進搜尋清單並標示 is_bound=True。
    """
    current_net = _current_ipv4_network()

    # 先整理掃描結果
    search_results = _normalize_search_rows(raw_search_results)

    # 追蹤是否已出現過（避免重複）
    seen_pairs = set()
    merged = []

    for item in search_results:
        ip_value = item.get("ip") or ""
        mac_value = (item.get("mac") or "").upper()

        bound_info = None
        if ip_value and ip_value in bound_map_by_ip:
            bound_info = bound_map_by_ip[ip_value]
        elif mac_value and mac_value in bound_map_by_mac:
            bound_info = bound_map_by_mac[mac_value]

        item["is_bound"] = bound_info is not None
        # 單純依掃描結果：屬於目前區網，直接保留
        merged.append(item)
        seen_pairs.add((ip_value, mac_value))

    # 把同區網、但這次沒掃描到的『已綁定』也補進上方清單
    for b in bound:
        ip_b = b.get("ip") or ""
        mac_b = (b.get("mac") or "").upper()
        if (ip_b, mac_b) in seen_pairs:
            continue
        # 僅補入與目前 IPv4 網段相同的
        try:
            if not ip_b or current_net is None:
                continue
            addr = ip_address(ip_b)
            if not isinstance(addr, IPv4Address):
                continue
            if addr not in current_net:
                continue
        except Exception:
            continue

        merged.append({
            "ip": ip_b,
            "mac": mac_b,
            "name": b.get("name") or "IPC",
            "is_bound": True,
        })
        seen_pairs.add((ip_b, mac_b))

    # 排序：未綁定在前、已綁定在後，其後以 IP 與 MAC 排序
    merged.sort(
        key=lambda item: (
            1 if item.get("is_bound") else 0,
            _ip_sort_key(item.get("ip")),
            item.get("mac") or "",
        )
    )
    return merged


def _fetch_scan_rows():
    """呼叫 IPC 掃描服務，並盡量從回應中提取裝置列表。"""
    try:
        payload = scan_ipc_dynamic()
    except Exception as exc:  # noqa: BLE001
        print("IPC scan call failed:", exc)
        return []

    candidates = []
    if isinstance(payload, list):
        candidates = payload
    elif isinstance(payload, dict):
        for key in ("devices", "device_list", "items", "list", "ipcs", "data"):
            value = payload.get(key)
            if isinstance(value, list):
                candidates = value
                break
        else:
            candidates = [payload]
    elif isinstance(payload, str):
        # 非 JSON 格式時，以換行分隔建立簡易結果。
        lines = [line.strip() for line in payload.splitlines() if line.strip()]
        for line in lines:
            segments = [seg.strip() for seg in re.split(r"[,\s]+", line) if seg.strip()]
            if segments:
                candidates.append({"ip": segments[0], "mac": segments[1] if len(segments) > 1 else ""})

    return candidates


class PreviewStreamError(Exception):
    """Raised when RTSP 預覽初始化失敗。"""

    def __init__(self, message: str, *, stderr: str = "", code: str | None = None):
        super().__init__(message)
        self.stderr = stderr
        self.code = code


def _describe_preview_error(
    stderr: str,
    *,
    default_message: str = "無法連接攝影機",
    code: str | None = None,
) -> tuple[str, str]:
    """將 ffmpeg 錯誤訊息轉為使用者可理解的描述。"""

    lowered = (stderr or "").lower()
    auth_tokens = ("unauthorized", "authentication", "password", "auth", "401")
    timeout_tokens = ("timed out", "timeout", "10060", "10061")
    connect_tokens = (
        "connection refused",
        "no route",
        "network is unreachable",
        "host is unreachable",
        "failed to resolve",
        "name or service not known",
        "connection failure",
        "unable to connect",
        "refused",
        "not found",
    )
    encoder_missing_tokens = (
        "unknown encoder 'libx264'",
        "encoder 'libx264' not found",
        "libx264 not found",
    )

    if code is None:
        if any(token in lowered for token in auth_tokens):
            return "需要輸入攝影機帳號或密碼", "AUTH_REQUIRED"
        if any(token in lowered for token in timeout_tokens):
            return "攝影機連線逾時", "TIMEOUT"
        if any(token in lowered for token in connect_tokens):
            return "無法連接攝影機", "CONNECTION_FAILED"
        if any(token in lowered for token in encoder_missing_tokens):
            return "系統缺少 H.264 編碼器 (libx264)。", "ENCODER_MISSING"

    if code == "MISSING_DEP":
        return "系統找不到 ffmpeg，無法建立預覽。", code
    if code == "WEBRTC_DISABLED":
        return "WebRTC 模組未啟用。", code
    if code == "ENCODER_MISSING":
        return "系統缺少 H.264 編碼器 (libx264)。", code
    if code == "TIMEOUT":
        return "攝影機連線逾時", code
    if code == "AUTH_REQUIRED":
        return "需要輸入攝影機帳號或密碼", code
    if code == "BAD_OFFER":
        return "無效的 SDP 參數", code
    if code:
        clean_default = default_message.strip() if default_message else "無法連接攝影機"
        return clean_default, code

    clean_message = default_message.strip() if default_message else "無法連接攝影機"
    return clean_message, "STREAM_ERROR"


def _preview_error_status(code: str | None) -> int:
    if code in {"MISSING_DEP", "ENCODER_MISSING", "WEBRTC_DISABLED"}:
        return 500
    if code == "BAD_OFFER":
        return 400
    if code == "AUTH_REQUIRED":
        return 401
    if code == "TIMEOUT":
        return 504
    return 502


def _build_rtsp_url(ip_value: str, account: str, password: str) -> str:
    """依輸入參數組出 rtsp:// URL，必要時進行驗證。"""

    ip_clean = (ip_value or "").strip()
    if not ip_clean:
        raise ValueError("缺少攝影機 IP 位址")
    try:
        addr = ip_address(ip_clean)
    except ValueError as exc:
        raise ValueError("IP 位址格式不正確") from exc

    host = ip_clean
    if isinstance(addr, IPv6Address):
        host = f"[{addr.compressed}]"

    account_value = account or ""
    password_value = password or ""
    credentials = ""
    if account_value or password_value:
        credentials = f"{quote(account_value, safe='')}:{quote(password_value, safe='')}@"

    return f"rtsp://{credentials}{host}:554/stream1"


_preview_loop: Optional[asyncio.AbstractEventLoop] = None
_preview_thread: Optional[threading.Thread] = None
_preview_sessions: Dict[str, Dict[str, Any]] = {}
_preview_media_relay: Optional[MediaRelay] = MediaRelay() if AIORTC_AVAILABLE else None
_preview_session_ttl_seconds = 180


def _ensure_preview_backend() -> None:
    if not AIORTC_AVAILABLE:
        raise PreviewStreamError("WebRTC 模組未啟用", code="WEBRTC_DISABLED")

    global _preview_loop, _preview_thread
    if _preview_loop is not None:
        return

    loop = asyncio.new_event_loop()

    def _run_loop() -> None:
        asyncio.set_event_loop(loop)
        loop.run_forever()

    thread = threading.Thread(target=_run_loop, name="webrtc-preview-loop", daemon=True)
    thread.start()

    _preview_loop = loop
    _preview_thread = thread


def _run_in_preview_loop(coro: Coroutine[Any, Any, Any]) -> Any:
    _ensure_preview_backend()
    assert _preview_loop is not None
    future = asyncio.run_coroutine_threadsafe(coro, _preview_loop)
    return future.result()


async def _close_preview_session(session_id: str) -> bool:
    session = _preview_sessions.pop(session_id, None)
    if not session:
        return False

    pc: Optional[RTCPeerConnection] = session.get("pc") if AIORTC_AVAILABLE else None
    player = session.get("player")

    if player and hasattr(player, "stop"):
        try:
            await player.stop()
        except Exception:  # pragma: no cover - best effort cleanup
            logger.debug("Failed to stop media player for session %s", session_id, exc_info=True)

    if pc:
        try:
            await pc.close()
        except Exception:  # pragma: no cover
            logger.debug("Failed to close peer connection for session %s", session_id, exc_info=True)

    return True


async def _session_watchdog(session_id: str, ttl: int) -> None:
    try:
        await asyncio.sleep(ttl)
        await _close_preview_session(session_id)
    except Exception:  # pragma: no cover - watchdog best effort
        logger.debug("Watchdog cleanup failed for session %s", session_id, exc_info=True)


async def _create_webrtc_session(rtsp_url: str) -> tuple[str, RTCPeerConnection]:
    if not AIORTC_AVAILABLE or RTCPeerConnection is None or MediaPlayer is None:
        raise PreviewStreamError("WebRTC 模組未啟用", code="WEBRTC_DISABLED")

    session_id = secrets.token_urlsafe(12)
    pc = RTCPeerConnection()

    media_options = {
        "rtsp_transport": "tcp",
        "stimeout": "5000000",
        "fflags": "nobuffer",
        "flags": "low_delay",
    }

    try:
        player = MediaPlayer(rtsp_url, format="rtsp", options=media_options)
    except Exception as exc:  # pragma: no cover - depends on camera availability
        await pc.close()
        raise PreviewStreamError("無法建立攝影機串流", stderr=str(exc)) from exc

    video_track = getattr(player, "video", None)
    if video_track is None:
        await player.stop()
        await pc.close()
        raise PreviewStreamError("攝影機未提供視訊串流")

    relay = _preview_media_relay or MediaRelay()
    pc.addTrack(relay.subscribe(video_track))

    @pc.on("connectionstatechange")
    async def _on_connection_change() -> None:
        if pc.connectionState in {"failed", "closed"}:
            await _close_preview_session(session_id)

    _preview_sessions[session_id] = {"pc": pc, "player": player}

    asyncio.ensure_future(_session_watchdog(session_id, _preview_session_ttl_seconds))

    return session_id, pc


async def _handle_preview_offer(rtsp_url: str, offer_dict: Dict[str, str]) -> tuple[str, RTCSessionDescription]:
    session_id, pc = await _create_webrtc_session(rtsp_url)

    try:
        offer = RTCSessionDescription(sdp=offer_dict["sdp"], type=offer_dict["type"])
    except Exception as exc:
        await _close_preview_session(session_id)
        raise PreviewStreamError("無效的 SDP 參數", stderr=str(exc), code="BAD_OFFER") from exc

    ice_complete = asyncio.Event()

    @pc.on("icegatheringstatechange")
    async def _on_ice_state_change() -> None:
        if pc.iceGatheringState == "complete":
            ice_complete.set()

    try:
        await pc.setRemoteDescription(offer)
        answer = await pc.createAnswer()
        await pc.setLocalDescription(answer)

        if pc.iceGatheringState != "complete":
            try:
                await asyncio.wait_for(ice_complete.wait(), timeout=3.0)
            except asyncio.TimeoutError:
                pass
    except Exception as exc:
        await _close_preview_session(session_id)
        raise PreviewStreamError("無法建立 WebRTC 連線", stderr=str(exc)) from exc

    assert pc.localDescription is not None
    return session_id, pc.localDescription


@_require_auth
@require_http_methods(["POST"])
def api_cameras_preview_probe(request: HttpRequest):
    """先行檢查 RTSP 串流是否可連線，失敗時回傳提示訊息。"""

    try:
        payload = json.loads(request.body.decode("utf-8"))
    except Exception:
        payload = {}

    ip_value = (payload.get("ip") or "").strip()
    account_value = payload.get("account") or ""
    password_value = payload.get("password") or ""

    try:
        rtsp_url = _build_rtsp_url(ip_value, account_value, password_value)
    except ValueError as exc:
        return JsonResponse({"ok": False, "error": str(exc), "code": "BAD_INPUT"}, status=400)

    command = [
        "ffmpeg",
        "-hide_banner",
        "-loglevel", "error",
        "-rtsp_transport", "tcp",
        "-i", rtsp_url,
        "-frames:v", "1",
        "-f", "null",
        "-",
    ]

    try:
        completed = subprocess.run(
            command,
            check=False,
            capture_output=True,
            timeout=8,
        )
    except FileNotFoundError:
        return JsonResponse(
            {
                "ok": False,
                "error": "系統找不到 ffmpeg，無法建立預覽。",
                "code": "MISSING_DEP",
            },
            status=500,
        )
    except subprocess.TimeoutExpired:
        return JsonResponse(
            {"ok": False, "error": "攝影機連線逾時", "code": "TIMEOUT"},
            status=504,
        )

    if completed.returncode != 0:
        stderr_text = completed.stderr.decode("utf-8", errors="ignore")
        message, code = _describe_preview_error(stderr_text)
        status_code = 401 if code == "AUTH_REQUIRED" else 504 if code == "TIMEOUT" else 502
        return JsonResponse({"ok": False, "error": message, "code": code}, status=status_code)

    return JsonResponse({"ok": True})


@_require_auth
@require_http_methods(["POST"])
def api_cameras_preview_webrtc_offer(request: HttpRequest):
    if not AIORTC_AVAILABLE:
        return JsonResponse(
            {
                "ok": False,
                "error": "此環境尚未安裝 WebRTC 模組 (aiortc)。",
                "code": "WEBRTC_DISABLED",
            },
            status=500,
        )

    try:
        payload = json.loads(request.body.decode("utf-8"))
    except Exception:
        return JsonResponse({"ok": False, "error": "Invalid JSON"}, status=400)

    ip_value = (payload.get("ip") or "").strip()
    account_value = payload.get("account") or ""
    password_value = payload.get("password") or ""
    offer_data = payload.get("offer") or {}

    if not isinstance(offer_data, dict) or "sdp" not in offer_data or "type" not in offer_data:
        return JsonResponse({"ok": False, "error": "Missing SDP offer"}, status=400)

    try:
        rtsp_url = _build_rtsp_url(ip_value, account_value, password_value)
    except ValueError as exc:
        return JsonResponse({"ok": False, "error": str(exc), "code": "BAD_INPUT"}, status=400)

    try:
        session_id, local_desc = _run_in_preview_loop(_handle_preview_offer(rtsp_url, offer_data))
    except PreviewStreamError as exc:
        message, code = _describe_preview_error(exc.stderr, default_message=str(exc), code=exc.code)
        status_code = _preview_error_status(code)
        return JsonResponse({"ok": False, "error": message, "code": code}, status=status_code)
    except Exception as exc:  # pragma: no cover - defensive path
        logger.exception("WebRTC offer handling failed")
        return JsonResponse({"ok": False, "error": str(exc) or "SERVER_ERROR", "code": "SERVER_ERROR"}, status=500)

    answer_payload = {"type": local_desc.type, "sdp": local_desc.sdp}
    return JsonResponse({"ok": True, "session_id": session_id, "answer": answer_payload})


@_require_auth
@require_http_methods(["POST"])
def api_cameras_preview_webrtc_hangup(request: HttpRequest):
    if not AIORTC_AVAILABLE:
        return JsonResponse({"ok": True, "detail": "WEBRTC_DISABLED"})

    try:
        payload = json.loads(request.body.decode("utf-8"))
    except Exception:
        payload = {}

    session_id = (payload.get("session_id") or "").strip()
    if not session_id:
        return JsonResponse({"ok": False, "error": "Missing session_id"}, status=400)

    try:
        closed = _run_in_preview_loop(_close_preview_session(session_id))
    except PreviewStreamError as exc:
        message, code = _describe_preview_error(exc.stderr, default_message=str(exc), code=exc.code)
        status_code = _preview_error_status(code)
        return JsonResponse({"ok": False, "error": message, "code": code}, status=status_code)
    except Exception as exc:  # pragma: no cover
        logger.exception("WebRTC hangup failed")
        return JsonResponse({"ok": False, "error": str(exc) or "SERVER_ERROR"}, status=500)

    if not closed:
        return JsonResponse({"ok": True, "detail": "SESSION_NOT_FOUND"})
    return JsonResponse({"ok": True})


@_require_auth
@require_http_methods(["GET"])
def cameras(request: HttpRequest):
    """攝影機管理頁面：上方為搜尋結果，下方為已綁定清單（讀取外部資料庫 connected_ipc）。"""
    bound, _, _ = _load_bound_entries()
    context = {"bound": bound, "search_results": []}
    return render(request, "ui/cameras_combined.html", context)


@_require_auth
@require_http_methods(["GET"])
def api_cameras_scan(request: HttpRequest):
    """提供即時掃描結果給前端查詢。"""
    bound, bound_map_by_ip, bound_map_by_mac = _load_bound_entries()
    raw_search_results = _fetch_scan_rows()
    search_results = _merge_scan_with_bound(
        bound,
        bound_map_by_ip,
        bound_map_by_mac,
        raw_search_results,
    )
    return JsonResponse({"ok": True, "results": search_results})




# ui/views.py
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
import json
import psycopg2
from psycopg2 import errors

DB_CONFIG = {
    "dbname": "redsafedb",
    "user": "redsafedb",
    "password": "redsafedb",
    "host": "127.0.0.1",
    "port": "5432"
}

@csrf_exempt
def api_cameras_bind(request):
    print("==== 接收到綁定請求 ====")
    if request.method == "POST":
        try:
            data = json.loads(request.body.decode())
        except json.JSONDecodeError:
            return JsonResponse({"ok": False, "error": "Invalid JSON"})

        ip_address = (data.get("ip_address") or "").strip()
        mac_address = (data.get("mac_address") or "").replace("-", ":").strip().upper()
        ipc_name = (data.get("ipc_name") or "").strip()
        custom_name = (data.get("custom_name") or "").strip()
        ipc_account = (data.get("ipc_account") or "").strip()
        ipc_password_raw = data.get("ipc_password")
        ipc_password = ""
        if isinstance(ipc_password_raw, str):
            ipc_password = ipc_password_raw
        elif ipc_password_raw is not None:
            ipc_password = str(ipc_password_raw)

        if not custom_name:
            custom_name = ipc_name
        if not custom_name:
            custom_name = "IPC"
        if not ipc_name:
            ipc_name = custom_name

        account_value = ipc_account or None
        password_value = ipc_password if ipc_password else None

        if not ip_address or not mac_address:
            return JsonResponse({"ok": False, "error": "Missing required fields"})

        conn = None
        cur = None
        try:
            conn = psycopg2.connect(**DB_CONFIG)
            cur = conn.cursor()
            cur.execute(
                """
                SELECT 1 FROM connected_ipc
                WHERE ip_address = %s OR mac_address = %s
                LIMIT 1
                """,
                (ip_address, mac_address),
            )
            if cur.fetchone():
                # fetch the existing row for UI to reflect state without reload
                try:
                    cur.execute(
                        """
                        SELECT
                            ip_address,
                            mac_address,
                            COALESCE(NULLIF(custom_name, ''), NULLIF(ipc_name, ''), mac_address) AS name
                        FROM connected_ipc
                        WHERE ip_address = %s OR mac_address = %s
                        LIMIT 1
                        """,
                        (ip_address, mac_address),
                    )
                    row = cur.fetchone()
                except Exception:
                    row = None
                item = None
                if row:
                    item = {
                        "ip": (row[0] or "").strip(),
                        "mac": (row[1] or "").replace("-", ":").strip().upper(),
                        "name": (row[2] or "IPC"),
                        "is_bound": True
                    }
                return JsonResponse({"ok": False, "error": "攝影機已綁定", "code": "ALREADY_BOUND", "item": item})

            cur.execute(
                """
                INSERT INTO connected_ipc (ip_address, mac_address, ipc_name, custom_name, ipc_account, ipc_password)
                VALUES (%s, %s, %s, %s, %s, %s)
                """,
                (
                    ip_address,
                    mac_address,
                    ipc_name,
                    custom_name,
                    account_value,
                    password_value,
                ),
            )
            conn.commit()
            return JsonResponse({
                "ok": True,
                "item": {
                    "ip": ip_address,
                    "mac": mac_address,
                    "name": custom_name,
                    "is_bound": True
                }
            })
        except errors.UniqueViolation:
            if conn:
                conn.rollback()
            # fetch the existing row for UI to reflect state without reload
            try:
                if cur is None:
                    cur = conn.cursor()
                cur.execute(
                    """
                    SELECT
                        ip_address,
                        mac_address,
                        COALESCE(NULLIF(custom_name, ''), NULLIF(ipc_name, ''), mac_address) AS name
                    FROM connected_ipc
                    WHERE ip_address = %s OR mac_address = %s
                    LIMIT 1
                    """,
                    (ip_address, mac_address),
                )
                row = cur.fetchone()
            except Exception:
                row = None
            item = None
            if row:
                item = {
                    "ip": (row[0] or "").strip(),
                    "mac": (row[1] or "").replace("-", ":").strip().upper(),
                    "name": (row[2] or "IPC"),
                    "is_bound": True
                }
            return JsonResponse({"ok": False, "error": "攝影機已綁定", "code": "ALREADY_BOUND", "item": item})
        except Exception as e:
            if conn:
                conn.rollback()
            print("DB insert error:", e)
            return JsonResponse({"ok": False, "error": str(e)})
        finally:
            if cur:
                cur.close()
            if conn:
                conn.close()
    return JsonResponse({"ok": False, "error": "Invalid method"})

@csrf_exempt
def api_cameras_unbind(request):
    print("==== 接收到刪除請求 ====")
    if request.method == "POST":
        data = json.loads(request.body.decode())
        mac_address = (data.get("mac_address") or "").replace("-", ":").strip().upper()
        ip_address = (data.get("ip_address") or "").strip()
        if not mac_address and not ip_address:
            return JsonResponse({"ok": False, "error": "Missing MAC or IP address"})
        try:
            conn = psycopg2.connect(**DB_CONFIG)
            cur = conn.cursor()
            if mac_address:
                cur.execute(
                    "DELETE FROM connected_ipc WHERE mac_address = %s RETURNING ip_address, mac_address",
                    (mac_address,)
                )
            else:
                cur.execute(
                    "DELETE FROM connected_ipc WHERE ip_address = %s RETURNING ip_address, mac_address",
                    (ip_address,)
                )
            deleted = cur.fetchone()
            conn.commit()
            cur.close()
            conn.close()
            if not deleted:
                return JsonResponse({"ok": False, "error": "NOT_FOUND"})
            return JsonResponse({
                "ok": True,
                "item": {
                    "ip": (deleted[0] or "").strip(),
                    "mac": (deleted[1] or "").replace("-", ":").strip().upper()
                }
            })
        except Exception as e:
            print("DB delete error:", e)
            return JsonResponse({"ok": False, "error": str(e)})
    return JsonResponse({"ok": False, "error": "Invalid method"})

# ====== API（啟用；登入保護） ======
@_require_auth
@require_http_methods(["GET"])
def api_setupconfig_list(request: HttpRequest):
    """示範回傳 config 表資料內容供前端檢視。"""

    data = list(EdgeConfig.objects.all().values("edge_id", "edge_password"))
    return JsonResponse({"items": data, "count": len(data)})


# 新增：回傳目前已綁定清單（下方表單）給前端即時刷新用，避免整頁重新整理。
@_require_auth
@require_http_methods(["GET"])
def api_cameras_bound_list(request: HttpRequest):
    """
    回傳目前已綁定清單（下方表單）給前端即時刷新用，避免整頁重新整理。
    """
    bound, _, _ = _load_bound_entries()
    # 保持與搜尋清單相同的欄位結構
    items = [
        {
            "ip": b.get("ip") or "",
            "mac": (b.get("mac") or "").upper(),
            "name": b.get("name") or "IPC",
            "is_bound": True,
        }
        for b in bound
    ]
    return JsonResponse({"ok": True, "items": items, "count": len(items)})
