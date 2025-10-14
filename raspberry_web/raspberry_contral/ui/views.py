"""UI 應用的主要視圖與 API。"""

import base64
import io
import json
import re
import subprocess
from ipaddress import IPv4Address, IPv6Address, IPv4Network, ip_address, ip_network
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

from .models import ConnectedIPC, EdgeConfig


def show_users(request):
    """顯示 config 表中的所有資料列供前端檢視。"""

    rows = EdgeConfig.objects.all()
    return render(request, "ui/users.html", {"rows": rows})


SESSION_KEY = "setup_authed"

# 密碼規則：僅英數、至少 6 碼
PASSWORD_REGEX = re.compile(r'^[A-Za-z0-9]{6,}$')

DEFAULT_EDGE_VERSION = getattr(settings, "EDGE_VERSION", "1.0.0")


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
    """回傳綁定使用者清單，目前未提供資料則回傳空集合。"""

    return JsonResponse({"items": [], "count": 0})


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
def api_user_remove(request, user_id):
    """目前未支援刪除綁定使用者，統一回傳錯誤。"""

    return JsonResponse(
        {"status": "error", "message": "目前未支援綁定使用者管理"},
        status=400,
    )


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
            name_value = (r.custom_name or r.ipc_name or r.mac_address or "IPC")
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
        ipc_name = data.get("ipc_name")
        ipc_account = data.get("ipc_account")
        ipc_password = data.get("ipc_password")

        # normalize empty strings to None-like for consistent handling
        if not ipc_name:
            ipc_name = ""
        if not ipc_account:
            ipc_account = ""
        if not ipc_password:
            ipc_password = ""

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
                        "SELECT ip_address, mac_address, COALESCE(custom_name, ipc_name, mac_address) AS name FROM connected_ipc WHERE ip_address = %s OR mac_address = %s LIMIT 1",
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
                    ipc_name,
                    ipc_account,
                    ipc_password,
                ),
            )
            conn.commit()
            return JsonResponse({
                "ok": True,
                "item": {
                    "ip": ip_address,
                    "mac": mac_address,
                    "name": ipc_name or (ipc_account or "IPC"),
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
                    "SELECT ip_address, mac_address, COALESCE(custom_name, ipc_name, mac_address) AS name FROM connected_ipc WHERE ip_address = %s OR mac_address = %s LIMIT 1",
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
