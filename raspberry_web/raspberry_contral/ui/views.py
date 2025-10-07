"""UI 應用的主要視圖與 API。"""

import base64
import io
import re
import json
import subprocess

import netifaces
import psutil
import qrcode
import requests
from django.conf import settings
from django.http import HttpRequest, HttpResponse, JsonResponse
from django.middleware.csrf import get_token
from django.shortcuts import redirect, render
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_http_methods
from ipcscan_client.ipcsacn import scan_ipc_dynamic

from .models import EdgeConfig


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

    # 若未接實際裝置資料，先使用預設（題主要求）
    serial = scan_ipc_dynamic()  # 預留：序號（目前固定為 12345678）
    version = "v1.0.0"  # 預留：版本
    status = 1  # 1 = 已連線, 0 = 未連線
    password = "12345678"  # 預留密碼（題主要求預設為 12345678）
    name = "裝置"

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


# ====== API（啟用；登入保護） ======
@_require_auth
@require_http_methods(["GET"])
def api_setupconfig_list(request: HttpRequest):
    """示範回傳 config 表資料內容供前端檢視。"""

    data = list(EdgeConfig.objects.all().values("edge_id", "edge_password"))
    return JsonResponse({"items": data, "count": len(data)})
