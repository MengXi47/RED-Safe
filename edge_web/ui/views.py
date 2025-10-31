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


def _normalize_fall_sensitivity(value: Any, default: int = 70) -> int:
    """確保靈敏度值維持在 0-100 範圍內。"""
    try:
        numeric = int(value)
    except (TypeError, ValueError):
        numeric = default
    return max(0, min(100, numeric))


def _get_edge_config() -> EdgeConfig | None:
    """取得唯一一筆 edge 設定資料。"""

    return EdgeConfig.objects.first()


def _build_device_snapshot() -> dict[str, Any]:
    """根據本地資料庫組裝裝置資訊快照。"""

    config = _get_edge_config()
    serial = config.edge_id if config and config.edge_id else "RED-UNKNOWN"
    password = (config.edge_password or "") if config else ""

    raw_version = str(DEFAULT_EDGE_VERSION or "1.0.0")
    version = raw_version if raw_version.lower().startswith("v") else f"v{raw_version}"
    status = 1 if config else 0
    name = "裝置"
    has_password = bool(password)
    masked_password = "＊" * len(password) if has_password else "******"

    payload_dict = {"serial": serial, "password": password, "name": name}
    payload = json.dumps(payload_dict, ensure_ascii=False, separators=(",", ":"))

    qrcode_data: str | None
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
    except Exception as e:  # pragma: no cover - 資料夾缺少 qrcode 依賴時容錯
        qrcode_data = None
        logger.warning("QR generation error: %s", e)

    return {
        "serial": serial,
        "version": version,
        "status": status,
        "password": password,
        "masked_password": masked_password,
        "has_password": has_password,
        "qrcode": qrcode_data,
    }


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
            _log_web_action(
                request,
                action="user_login_failed",
                message="嘗試登入失敗：密碼格式不符合規範",
                level="WARN",
                metadata={"reason": "invalid_format"},
            )
            return render(request, "ui/login.html", ctx)

        if pwd == (config.edge_password or ""):
            request.session[SESSION_KEY] = True
            _log_web_action(request, action="user_login", message="使用者透過網頁介面登入系統")
            return redirect("dashboard")

        _log_web_action(
            request,
            action="user_login_failed",
            message="嘗試登入失敗：密碼錯誤",
            level="WARN",
            metadata={"reason": "wrong_password"},
        )
        ctx["error"] = "密碼錯誤。"

    return render(request, "ui/login.html", ctx)


def logout_view(request: HttpRequest):
    """登出並回到登入頁面。"""

    request.session.pop(SESSION_KEY, None)
    _log_web_action(request, action="user_logout", message="使用者自網頁介面登出")
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

    initial_state = {
        "network": {
            "ip_address": ip_address,
            "netmask": netmask,
            "gateway": gateway,
            "dns": dns,
        }
    }
    return render(request, "ui/network_ip.html", {"initial_state": initial_state})


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
        _log_web_action(
            request,
            action="user_unbind_failed",
            message="解除使用者綁定失敗：edge_id 未設定",
            level="ERROR",
        )
        return JsonResponse(
            {"status": "error", "message": "edge_id_not_configured"},
            status=400,
        )

    try:
        payload = json.loads(request.body or "{}")
    except json.JSONDecodeError:
        _log_web_action(
            request,
            action="user_unbind_failed",
            message="解除使用者綁定失敗：JSON 格式錯誤",
            level="WARN",
        )
        return JsonResponse(
            {"status": "error", "message": "invalid_json"},
            status=400,
        )

    email = str(payload.get("email", "")).strip()
    if not email:
        _log_web_action(
            request,
            action="user_unbind_failed",
            message="解除使用者綁定失敗：缺少 Email",
            level="WARN",
        )
        return JsonResponse(
            {"status": "error", "message": "email_required"},
            status=400,
        )

    api_result = remove_bound_user(edge_id, email)
    if not isinstance(api_result, dict):
        _log_web_action(
            request,
            action="user_unbind_failed",
            message="解除使用者綁定失敗：雲端回傳格式異常",
            level="ERROR",
            metadata={"email": email},
        )
        return JsonResponse(
            {"status": "error", "message": "unexpected_response"},
            status=502,
        )

    if "error" in api_result:
        status_code = api_result.get("status")
        if not isinstance(status_code, int) or status_code < 400:
            status_code = 502
        detail = {k: v for k, v in api_result.items() if k != "error"}
        _log_web_action(
            request,
            action="user_unbind_failed",
            message=f"解除使用者綁定失敗：{api_result['error']}",
            level="WARN",
            metadata={"email": email, "detail": detail},
        )
        return JsonResponse(
            {"status": "error", "message": api_result["error"], "detail": detail},
            status=status_code,
        )

    error_code = str(api_result.get("error_code", ""))
    if error_code != "0":
        _log_web_action(
            request,
            action="user_unbind_failed",
            message="解除使用者綁定失敗：遠端錯誤",
            level="WARN",
            metadata={"email": email, "error_code": error_code},
        )
        return JsonResponse(
            {"status": "error", "message": error_code or "unknown_error"},
            status=400,
        )

    _log_web_action(
        request,
        action="user_unbind_success",
        message="解除使用者綁定成功",
        metadata={"email": email},
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
                error_code = str(data.get("error_code", ""))
                if resp.status_code == 200 and error_code == "0":
                    config.edge_password = new1
                    config.save(update_fields=["edge_password"])
                    request.session.flush()
                    _log_web_action(
                        request,
                        action="device_password_change_success",
                        message="變更裝置密碼成功並已登出使用者",
                    )
                    return redirect("login")
                if resp.status_code == 401 or error_code == "147":
                    errors["old_password"] = "遠端驗證失敗，請確認舊密碼"
                else:
                    errors["new_password2"] = (
                        f"遠端更新失敗：HTTP {resp.status_code} code={error_code or '未知'}"
                    )
            except requests.RequestException as exc:
                errors["new_password2"] = f"遠端連線失敗：{exc}"

    if request.method == "POST" and errors:
        _log_web_action(
            request,
            action="device_password_change_failed",
            message="變更裝置密碼失敗",
            level="WARN",
            metadata={"error_fields": list(errors.keys()), "messages": errors},
        )

    return render(
        request,
        "ui/device_change_password.html",
        {"initial_state": {"formErrors": errors}},
    )


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

    initial_state = {"users": []}
    return render(request, "ui/user_bound.html", {"initial_state": initial_state})


# 替換現有 device_info view 為下面版本
@_require_auth
def device_info(request):
    """顯示裝置基本資訊與預設 QR Code。"""

    snapshot = _build_device_snapshot()
    return render(request, "ui/device_info.html", {"initial_state": {"device": snapshot}})


@_require_auth
@require_http_methods(["GET"])
def api_device_info(request: HttpRequest):
    """回傳本地資料庫中的裝置基本資訊。"""

    snapshot = _build_device_snapshot()
    return JsonResponse({"device": snapshot})


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
            fall_sensitivity = _normalize_fall_sensitivity(r.fall_sensitivity)
            entry = {
                "ip": ip_value,
                "mac": mac_value,
                "name": name_value,
                "fall_sensitivity": fall_sensitivity,
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
        _log_web_action(
            request,
            action="camera_preview_probe_payload_error",
            message="攝影機預覽探測失敗：JSON 解析錯誤",
            level="WARN",
        )
        payload = {}

    ip_value = (payload.get("ip") or "").strip()
    account_value = payload.get("account") or ""
    password_value = payload.get("password") or ""
    probe_meta = {
        "camera_ip": ip_value,
        "has_account": bool(account_value),
        "has_password": bool(password_value),
    }

    try:
        rtsp_url = _build_rtsp_url(ip_value, account_value, password_value)
    except ValueError as exc:
        _log_web_action(
            request,
            action="camera_preview_probe_failed",
            message=f"攝影機預覽探測失敗：{exc}",
            level="WARN",
            metadata=probe_meta,
        )
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
        _log_web_action(
            request,
            action="camera_preview_probe_failed",
            message="攝影機預覽探測失敗：系統缺少 ffmpeg",
            level="ERROR",
            metadata=probe_meta,
        )
        return JsonResponse(
            {
                "ok": False,
                "error": "系統找不到 ffmpeg，無法建立預覽。",
                "code": "MISSING_DEP",
            },
            status=500,
        )
    except subprocess.TimeoutExpired:
        _log_web_action(
            request,
            action="camera_preview_probe_failed",
            message="攝影機預覽探測失敗：逾時",
            level="WARN",
            metadata=probe_meta,
        )
        return JsonResponse(
            {"ok": False, "error": "攝影機連線逾時", "code": "TIMEOUT"},
            status=504,
        )

    if completed.returncode != 0:
        stderr_text = completed.stderr.decode("utf-8", errors="ignore")
        message, code = _describe_preview_error(stderr_text)
        status_code = 401 if code == "AUTH_REQUIRED" else 504 if code == "TIMEOUT" else 502
        _log_web_action(
            request,
            action="camera_preview_probe_failed",
            message=f"攝影機預覽探測失敗：{message}",
            level="WARN",
            metadata={**probe_meta, "error_code": code},
        )
        return JsonResponse({"ok": False, "error": message, "code": code}, status=status_code)

    _log_web_action(
        request,
        action="camera_preview_probe_success",
        message="攝影機預覽探測成功",
        metadata=probe_meta,
    )
    return JsonResponse({"ok": True})


@_require_auth
@require_http_methods(["POST"])
def api_cameras_preview_webrtc_offer(request: HttpRequest):
    if not AIORTC_AVAILABLE:
        _log_web_action(
            request,
            action="camera_preview_webrtc_failed",
            message="攝影機 WebRTC 建立失敗：aiortc 未安裝",
            level="ERROR",
        )
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
        _log_web_action(
            request,
            action="camera_preview_webrtc_failed",
            message="攝影機 WebRTC 建立失敗：JSON 解析錯誤",
            level="WARN",
        )
        return JsonResponse({"ok": False, "error": "Invalid JSON"}, status=400)

    ip_value = (payload.get("ip") or "").strip()
    account_value = payload.get("account") or ""
    password_value = payload.get("password") or ""
    offer_data = payload.get("offer") or {}
    offer_meta = {
        "camera_ip": ip_value,
        "has_account": bool(account_value),
        "has_password": bool(password_value),
    }

    if not isinstance(offer_data, dict) or "sdp" not in offer_data or "type" not in offer_data:
        _log_web_action(
            request,
            action="camera_preview_webrtc_failed",
            message="攝影機 WebRTC 建立失敗：缺少 SDP 參數",
            level="WARN",
            metadata=offer_meta,
        )
        return JsonResponse({"ok": False, "error": "Missing SDP offer"}, status=400)

    try:
        rtsp_url = _build_rtsp_url(ip_value, account_value, password_value)
    except ValueError as exc:
        _log_web_action(
            request,
            action="camera_preview_webrtc_failed",
            message=f"攝影機 WebRTC 建立失敗：{exc}",
            level="WARN",
            metadata=offer_meta,
        )
        return JsonResponse({"ok": False, "error": str(exc), "code": "BAD_INPUT"}, status=400)

    try:
        session_id, local_desc = _run_in_preview_loop(_handle_preview_offer(rtsp_url, offer_data))
    except PreviewStreamError as exc:
        message, code = _describe_preview_error(exc.stderr, default_message=str(exc), code=exc.code)
        status_code = _preview_error_status(code)
        _log_web_action(
            request,
            action="camera_preview_webrtc_failed",
            message=f"攝影機 WebRTC 建立失敗：{message}",
            level="WARN",
            metadata={**offer_meta, "error_code": code},
        )
        return JsonResponse({"ok": False, "error": message, "code": code}, status=status_code)
    except Exception as exc:  # pragma: no cover - defensive path
        logger.exception("WebRTC offer handling failed")
        _log_web_action(
            request,
            action="camera_preview_webrtc_failed",
            message="攝影機 WebRTC 建立失敗：伺服器例外",
            level="ERROR",
            metadata={**offer_meta, "error": str(exc)},
        )
        return JsonResponse({"ok": False, "error": str(exc) or "SERVER_ERROR", "code": "SERVER_ERROR"}, status=500)

    answer_payload = {"type": local_desc.type, "sdp": local_desc.sdp}
    _log_web_action(
        request,
        action="camera_preview_webrtc_success",
        message="攝影機 WebRTC 建立成功",
        metadata={**offer_meta, "session_id": session_id},
    )
    return JsonResponse({"ok": True, "session_id": session_id, "answer": answer_payload})


@_require_auth
@require_http_methods(["POST"])
def api_cameras_preview_webrtc_hangup(request: HttpRequest):
    if not AIORTC_AVAILABLE:
        _log_web_action(
            request,
            action="camera_preview_hangup_skipped",
            message="攝影機 WebRTC 結束通知：aiortc 未安裝",
            level="WARN",
        )
        return JsonResponse({"ok": True, "detail": "WEBRTC_DISABLED"})

    try:
        payload = json.loads(request.body.decode("utf-8"))
    except Exception:
        _log_web_action(
            request,
            action="camera_preview_hangup_payload_error",
            message="攝影機 WebRTC 結束通知失敗：JSON 解析錯誤",
            level="WARN",
        )
        payload = {}

    session_id = (payload.get("session_id") or "").strip()
    if not session_id:
        _log_web_action(
            request,
            action="camera_preview_hangup_failed",
            message="攝影機 WebRTC 結束通知失敗：缺少 session_id",
            level="WARN",
        )
        return JsonResponse({"ok": False, "error": "Missing session_id"}, status=400)

    try:
        closed = _run_in_preview_loop(_close_preview_session(session_id))
    except PreviewStreamError as exc:
        message, code = _describe_preview_error(exc.stderr, default_message=str(exc), code=exc.code)
        status_code = _preview_error_status(code)
        _log_web_action(
            request,
            action="camera_preview_hangup_failed",
            message=f"攝影機 WebRTC 結束通知失敗：{message}",
            level="WARN",
            metadata={"session_id": session_id, "error_code": code},
        )
        return JsonResponse({"ok": False, "error": message, "code": code}, status=status_code)
    except Exception as exc:  # pragma: no cover
        logger.exception("WebRTC hangup failed")
        _log_web_action(
            request,
            action="camera_preview_hangup_failed",
            message="攝影機 WebRTC 結束通知失敗：伺服器例外",
            level="ERROR",
            metadata={"session_id": session_id, "error": str(exc)},
        )
        return JsonResponse({"ok": False, "error": str(exc) or "SERVER_ERROR"}, status=500)

    if not closed:
        _log_web_action(
            request,
            action="camera_preview_hangup_not_found",
            message="攝影機 WebRTC 結束通知：找不到 session",
            level="WARN",
            metadata={"session_id": session_id},
        )
        return JsonResponse({"ok": True, "detail": "SESSION_NOT_FOUND"})
    _log_web_action(
        request,
        action="camera_preview_hangup_success",
        message="攝影機 WebRTC 連線已結束",
        metadata={"session_id": session_id},
    )
    return JsonResponse({"ok": True})


@_require_auth
@require_http_methods(["GET"])
def cameras(request: HttpRequest):
    """攝影機管理頁面：上方為搜尋結果，下方為已綁定清單（讀取外部資料庫 connected_ipc）。"""
    bound, _, _ = _load_bound_entries()
    initial_state = {
        "cameras": {
            "bound": bound,
            "searchResults": [],
        }
    }
    return render(request, "ui/cameras_combined.html", {"initial_state": initial_state})


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
    _log_web_action(
        request,
        action="camera_scan",
        message="執行一次攝影機掃描",
        metadata={"result_count": len(search_results)},
    )
    return JsonResponse({"ok": True, "results": search_results})




# ui/views.py
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
import json
import psycopg2
from psycopg2 import errors
from psycopg2.extensions import connection as PgConnection, cursor as PgCursor
from psycopg2.extras import Json, RealDictCursor

_DEFAULT_DB = settings.DATABASES.get("default", {})
DB_CONFIG = {
    "dbname": (_DEFAULT_DB.get("NAME") or ""),
    "user": (_DEFAULT_DB.get("USER") or ""),
    "password": (_DEFAULT_DB.get("PASSWORD") or ""),
    "host": (_DEFAULT_DB.get("HOST") or "localhost"),
    "port": str(_DEFAULT_DB.get("PORT") or "5432"),
}

# 系統日誌設定：可依部署環境調整最大保留筆數，避免佔滿本地端儲存空間。
SYSTEM_LOG_TABLE = "web_system_logs"
MAX_SYSTEM_LOG_ROWS = int(getattr(settings, "SYSTEM_LOG_MAX_ROWS", 2000))
# 前端預設分頁大小，同樣提供中文註解方便後續調整與最佳化。
DEFAULT_SYSTEM_LOG_PAGE_SIZE = 50
MAX_SYSTEM_LOG_PAGE_SIZE = 200


def _ensure_system_log_table(conn: PgConnection) -> None:
    """建立系統日誌資料表與索引（若尚未存在）。"""

    with conn.cursor() as cur:
        cur.execute(
            f"""
            CREATE TABLE IF NOT EXISTS {SYSTEM_LOG_TABLE} (
                id BIGSERIAL PRIMARY KEY,
                category VARCHAR(64) NOT NULL DEFAULT 'web',
                action VARCHAR(128) NOT NULL,
                level VARCHAR(16) NOT NULL DEFAULT 'INFO',
                message TEXT NOT NULL,
                metadata JSONB,
                created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
            )
            """
        )
        cur.execute(
            f"CREATE INDEX IF NOT EXISTS idx_{SYSTEM_LOG_TABLE}_created_at ON {SYSTEM_LOG_TABLE} (created_at DESC, id DESC)"
        )
    conn.commit()


def _prune_system_logs(cur: PgCursor) -> None:
    """依照先進先出自動刪除超出上限的舊資料。"""

    cur.execute(f"SELECT COUNT(*) FROM {SYSTEM_LOG_TABLE}")
    total = cur.fetchone()
    count = int(total[0] if total else 0)
    overflow = count - MAX_SYSTEM_LOG_ROWS
    if overflow <= 0:
        return
    cur.execute(
        f"""
        DELETE FROM {SYSTEM_LOG_TABLE}
        WHERE id IN (
            SELECT id FROM {SYSTEM_LOG_TABLE}
            ORDER BY created_at ASC, id ASC
            LIMIT %s
        )
        """,
        (overflow,),
    )


def _normalize_system_log_row(row: dict[str, Any]) -> dict[str, Any]:
    """整理資料庫取出的列，確保 metadata 與時間格式一致。"""

    normalized = dict(row)
    metadata = normalized.get("metadata")
    if isinstance(metadata, memoryview):
        metadata = metadata.tobytes().decode("utf-8")
    if isinstance(metadata, str):
        try:
            metadata = json.loads(metadata)
        except Exception:
            pass
    normalized["metadata"] = metadata
    created_at = normalized.get("created_at")
    if hasattr(created_at, "isoformat"):
        normalized["created_at"] = created_at.isoformat()
    return normalized


def _write_system_log(
    action: str,
    message: str,
    *,
    level: str = "INFO",
    category: str = "web",
    metadata: Optional[dict[str, Any]] = None,
) -> None:
    """將 Web 端操作寫入系統日誌並觸發自動裁切。"""

    conn = None
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        _ensure_system_log_table(conn)
        with conn:
            with conn.cursor() as cur:
                cur.execute(
                    f"""
                    INSERT INTO {SYSTEM_LOG_TABLE} (category, action, level, message, metadata)
                    VALUES (%s, %s, %s, %s, %s)
                    """,
                    (
                        category,
                        action,
                        level.upper(),
                        message,
                        Json(metadata) if metadata is not None else None,
                    ),
                )
                _prune_system_logs(cur)
    except Exception:
        logger.exception("Failed to write system log")
    finally:
        if conn:
            conn.close()


def _client_ip(request: HttpRequest) -> str:
    """取得使用者來源 IP，優先採用 Proxy 標頭。"""

    forwarded = (request.META.get("HTTP_X_FORWARDED_FOR") or "").strip()
    if forwarded:
        return forwarded.split(",")[0].strip()
    return (request.META.get("REMOTE_ADDR") or "").strip()


def _request_context_metadata(request: HttpRequest) -> dict[str, str]:
    """整理請求環境資訊，供系統日誌參考。"""

    user_agent = (request.META.get("HTTP_USER_AGENT") or "").strip()
    if len(user_agent) > 255:
        user_agent = user_agent[:255]
    return {
        "ip": _client_ip(request),
        "user_agent": user_agent,
        "path": request.path,
    }


def _log_web_action(
    request: HttpRequest,
    action: str,
    message: str,
    *,
    level: str = "INFO",
    metadata: Optional[dict[str, Any]] = None,
) -> None:
    """統一記錄 Web 端動作，會自動帶入請求端環境資訊。"""

    base_meta: dict[str, Any] = dict(_request_context_metadata(request))
    if metadata:
        base_meta.update(metadata)
    _write_system_log(action=action, message=message, level=level, metadata=base_meta)


def _system_logs_handle_get(request: HttpRequest, conn: PgConnection) -> JsonResponse:
    """處理日誌列表查詢，支援分頁與簡易篩選。"""

    limit_param = request.GET.get("limit")
    offset_param = request.GET.get("offset")
    level_param = (request.GET.get("level") or "").strip().upper()
    category_param = (request.GET.get("category") or "").strip()

    try:
        limit = int(limit_param) if limit_param is not None else DEFAULT_SYSTEM_LOG_PAGE_SIZE
    except (TypeError, ValueError):
        limit = DEFAULT_SYSTEM_LOG_PAGE_SIZE
    limit = max(1, min(limit, MAX_SYSTEM_LOG_PAGE_SIZE))

    try:
        offset = int(offset_param) if offset_param is not None else 0
    except (TypeError, ValueError):
        offset = 0
    offset = max(0, offset)

    filters: list[str] = []
    params: list[Any] = []

    if level_param:
        filters.append("level = %s")
        params.append(level_param)
    if category_param:
        filters.append("category = %s")
        params.append(category_param)

    where_clause = f"WHERE {' AND '.join(filters)}" if filters else ""

    with conn.cursor() as cur:
        cur.execute(f"SELECT COUNT(*) FROM {SYSTEM_LOG_TABLE} {where_clause}", params)
        total_raw = cur.fetchone()
    total = int(total_raw[0] if total_raw else 0)

    query_params = list(params)
    query_params.extend([limit, offset])
    with conn.cursor(cursor_factory=RealDictCursor) as cur:
        cur.execute(
            f"""
            SELECT id, category, action, level, message, metadata, created_at
            FROM {SYSTEM_LOG_TABLE}
            {where_clause}
            ORDER BY created_at DESC, id DESC
            LIMIT %s OFFSET %s
            """,
            query_params,
        )
        rows = [_normalize_system_log_row(dict(row)) for row in cur.fetchall()]

    return JsonResponse(
        {
            "ok": True,
            "items": rows,
            "total": total,
            "limit": limit,
            "offset": offset,
            "max_rows": MAX_SYSTEM_LOG_ROWS,
        }
    )


def _system_logs_handle_post(request: HttpRequest, conn: PgConnection) -> JsonResponse:
    """處理新增日誌的請求並立即回傳新增資料。"""

    try:
        payload = json.loads(request.body.decode("utf-8"))
    except Exception:
        return JsonResponse({"ok": False, "error": "Invalid JSON"}, status=400)

    if not isinstance(payload, dict):
        return JsonResponse({"ok": False, "error": "Invalid payload"}, status=400)

    action = (payload.get("action") or "").strip()
    message = (payload.get("message") or "").strip()
    if not action or not message:
        return JsonResponse({"ok": False, "error": "Missing action or message"}, status=400)

    level = (payload.get("level") or "INFO").strip().upper() or "INFO"
    category = (payload.get("category") or "web").strip() or "web"
    metadata_raw = payload.get("metadata")
    if metadata_raw is not None and not isinstance(metadata_raw, (dict, list)):
        metadata_raw = None

    with conn:
        with conn.cursor(cursor_factory=RealDictCursor) as cur:
            cur.execute(
                f"""
                INSERT INTO {SYSTEM_LOG_TABLE} (category, action, level, message, metadata)
                VALUES (%s, %s, %s, %s, %s)
                RETURNING id, category, action, level, message, metadata, created_at
                """,
                (
                    category,
                    action,
                    level,
                    message,
                    Json(metadata_raw) if metadata_raw is not None else None,
                ),
            )
            inserted = cur.fetchone()
            _prune_system_logs(cur)

    if not inserted:
        return JsonResponse({"ok": False, "error": "Failed to insert log"}, status=500)

    return JsonResponse(
        {
            "ok": True,
            "item": _normalize_system_log_row(dict(inserted)),
            "max_rows": MAX_SYSTEM_LOG_ROWS,
        }
    )


@_require_auth
@require_http_methods(["GET", "POST"])
def api_system_logs(request: HttpRequest):
    """提供系統日誌查詢與新增功能，資料寫入本地 PostgreSQL。"""

    conn: PgConnection | None = None
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        _ensure_system_log_table(conn)
        if request.method == "GET":
            return _system_logs_handle_get(request, conn)
        return _system_logs_handle_post(request, conn)
    except Exception as exc:
        logger.exception("System log API error: {0}".format(exc))
        return JsonResponse({"ok": False, "error": "SERVER_ERROR"}, status=500)
    finally:
        if conn:
            conn.close()


@csrf_exempt
def api_cameras_bind(request):
    print("==== 接收到綁定請求 ====")
    if request.method == "POST":
        try:
            data = json.loads(request.body.decode())
        except json.JSONDecodeError:
            _log_web_action(
                request,
                action="camera_bind_failed",
                message="綁定攝影機失敗：JSON 格式錯誤",
                level="WARN",
            )
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

        fall_sensitivity = _normalize_fall_sensitivity(data.get("fall_sensitivity"))

        if not custom_name:
            custom_name = ipc_name
        if not custom_name:
            custom_name = "IPC"
        if not ipc_name:
            ipc_name = custom_name

        account_value = ipc_account or None
        password_value = ipc_password if ipc_password else None

        if not ip_address or not mac_address:
            _log_web_action(
                request,
                action="camera_bind_failed",
                message="綁定攝影機失敗：缺少 IP 或 MAC",
                level="WARN",
                metadata={
                    "camera_ip": ip_address,
                    "camera_mac": mac_address,
                },
            )
            return JsonResponse({"ok": False, "error": "Missing required fields"})

        log_metadata = {
            "camera_ip": ip_address,
            "camera_mac": mac_address,
            "ipc_name": ipc_name,
            "custom_name": custom_name,
            "fall_sensitivity": fall_sensitivity,
        }

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
                            COALESCE(NULLIF(custom_name, ''), NULLIF(ipc_name, ''), mac_address) AS name,
                            COALESCE(fall_sensitivity, 70) AS fall_sensitivity
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
                        "is_bound": True,
                        "fall_sensitivity": _normalize_fall_sensitivity(row[3]),
                    }
                _log_web_action(
                    request,
                    action="camera_bind_conflict",
                    message="綁定攝影機失敗：裝置已存在",
                    level="WARN",
                    metadata={**log_metadata, "name": item["name"] if item else custom_name},
                )
                return JsonResponse({"ok": False, "error": "攝影機已綁定", "code": "ALREADY_BOUND", "item": item})

            cur.execute(
                """
                INSERT INTO connected_ipc (ip_address, mac_address, ipc_name, custom_name, ipc_account, ipc_password, fall_sensitivity)
                VALUES (%s, %s, %s, %s, %s, %s, %s)
                """,
                (
                    ip_address,
                    mac_address,
                    ipc_name,
                    custom_name,
                    account_value,
                    password_value,
                    fall_sensitivity,
                ),
            )
            conn.commit()
            _log_web_action(
                request,
                action="camera_bind_success",
                message="綁定攝影機成功",
                metadata=log_metadata,
            )
            return JsonResponse({
                "ok": True,
                "item": {
                    "ip": ip_address,
                    "mac": mac_address,
                    "name": custom_name,
                    "is_bound": True,
                    "fall_sensitivity": fall_sensitivity,
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
                        COALESCE(NULLIF(custom_name, ''), NULLIF(ipc_name, ''), mac_address) AS name,
                        COALESCE(fall_sensitivity, 70) AS fall_sensitivity
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
                    "is_bound": True,
                    "fall_sensitivity": _normalize_fall_sensitivity(row[3]),
                }
            _log_web_action(
                request,
                action="camera_bind_conflict",
                message="綁定攝影機失敗：資料庫唯一值衝突",
                level="WARN",
                metadata={**log_metadata, "name": item["name"] if item else custom_name},
            )
            return JsonResponse({"ok": False, "error": "攝影機已綁定", "code": "ALREADY_BOUND", "item": item})
        except Exception as e:
            if conn:
                conn.rollback()
            print("DB insert error:", e)
            _log_web_action(
                request,
                action="camera_bind_failed",
                message="綁定攝影機失敗：資料庫錯誤",
                level="ERROR",
                metadata={**log_metadata, "error": str(e)},
            )
            return JsonResponse({"ok": False, "error": str(e)})
        finally:
            if cur:
                cur.close()
            if conn:
                conn.close()
    else:
        _log_web_action(
            request,
            action="camera_bind_invalid_method",
            message="綁定攝影機失敗：HTTP 方法不被允許",
            level="WARN",
        )
    return JsonResponse({"ok": False, "error": "Invalid method"})

@csrf_exempt
def api_cameras_unbind(request):
    print("==== 接收到刪除請求 ====")
    if request.method == "POST":
        try:
            data = json.loads(request.body.decode())
        except json.JSONDecodeError:
            _log_web_action(
                request,
                action="camera_unbind_failed",
                message="解除攝影機綁定失敗：JSON 格式錯誤",
                level="WARN",
            )
            return JsonResponse({"ok": False, "error": "Invalid JSON"})

        mac_address = (data.get("mac_address") or "").replace("-", ":").strip().upper()
        ip_address = (data.get("ip_address") or "").strip()
        if not mac_address and not ip_address:
            _log_web_action(
                request,
                action="camera_unbind_failed",
                message="解除攝影機綁定失敗：缺少辨識資訊",
                level="WARN",
                metadata={"camera_ip": ip_address, "camera_mac": mac_address},
            )
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
                _log_web_action(
                    request,
                    action="camera_unbind_not_found",
                    message="解除攝影機綁定失敗：找不到指定裝置",
                    level="WARN",
                    metadata={"camera_ip": ip_address, "camera_mac": mac_address},
                )
                return JsonResponse({"ok": False, "error": "NOT_FOUND"})
            _log_web_action(
                request,
                action="camera_unbind_success",
                message="解除攝影機綁定成功",
                metadata={
                    "camera_ip": (deleted[0] or "").strip(),
                    "camera_mac": (deleted[1] or "").replace("-", ":").strip().upper(),
                },
            )
            return JsonResponse({
                "ok": True,
                "item": {
                    "ip": (deleted[0] or "").strip(),
                    "mac": (deleted[1] or "").replace("-", ":").strip().upper()
                }
            })
        except Exception as e:
            print("DB delete error:", e)
            _log_web_action(
                request,
                action="camera_unbind_failed",
                message="解除攝影機綁定失敗：資料庫錯誤",
                level="ERROR",
                metadata={
                    "camera_ip": ip_address,
                    "camera_mac": mac_address,
                    "error": str(e),
                },
            )
            return JsonResponse({"ok": False, "error": str(e)})
    else:
        _log_web_action(
            request,
            action="camera_unbind_invalid_method",
            message="解除攝影機綁定失敗：HTTP 方法不被允許",
            level="WARN",
        )
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
            "fall_sensitivity": _normalize_fall_sensitivity(b.get("fall_sensitivity")),
        }
        for b in bound
    ]
    return JsonResponse({"ok": True, "items": items, "count": len(items)})
