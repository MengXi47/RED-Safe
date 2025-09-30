# ui/views.py
import re
import psutil
import subprocess
import netifaces
import socket
from django.shortcuts import render, redirect
from django.http import HttpRequest, HttpResponse, JsonResponse
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_http_methods
from django.middleware.csrf import get_token
from .models import SetupConfig, BoundUser
from django.shortcuts import get_object_or_404


SESSION_KEY = "setup_authed"

# 密碼規則：僅英數、至少 6 碼
PASSWORD_REGEX = re.compile(r'^[A-Za-z0-9]{6,}$')

def _is_configured() -> bool:
    return SetupConfig.objects.exists()

def _require_auth(view_func):
    """尚未設定 → /setup/；未登入 → /login/"""
    def wrapper(request: HttpRequest, *args, **kwargs):
        if not _is_configured():
            return redirect("setup")
        if not request.session.get(SESSION_KEY):
            return redirect("login")
        return view_func(request, *args, **kwargs)
    return wrapper

def _pi_temperature() -> str:
    """Read Raspberry Pi CPU temperature as a string like '52.1°C', or 'N/A'."""
    try:
        out = subprocess.check_output(["vcgencmd", "measure_temp"]).decode()
        return out.replace("temp=", "").replace("'C", "°C").strip()
    except Exception:
        try:
            with open("/sys/class/thermal/thermal_zone0/temp") as f:
                return f"{int(f.read())/1000:.1f}°C"
        except Exception:
            return "N/A"

# ====== 首次設定（只在未設定時可用） ======
@require_http_methods(["GET", "POST"])
def setup_view(request: HttpRequest):
    if _is_configured():
        return redirect("login")

    ctx = {"csrf_token": get_token(request), "error": None}
    if request.method == "POST":
        pwd = (request.POST.get("password") or "").strip()
        confirm = (request.POST.get("confirm") or "").strip()

        # 後端驗證（防繞過前端）
        if not PASSWORD_REGEX.fullmatch(pwd):
            ctx["error"] = "密碼需至少 6 碼，且僅能使用英文或數字（不可空白、中文或符號）。"
            return render(request, "ui/setup.html", ctx)

        if pwd != confirm:
            ctx["error"] = "兩次輸入不一致。"
            return render(request, "ui/setup.html", ctx)

        # 直接存明碼（注意：僅限受控內網環境）
        SetupConfig.objects.create(password_plain=pwd)
        request.session[SESSION_KEY] = True
        return redirect("dashboard")

    return render(request, "ui/setup.html", ctx)

# ====== 登入（之後每次都走這裡） ======
@require_http_methods(["GET", "POST"])
def login_view(request: HttpRequest):
    if not _is_configured():
        return redirect("setup")

    ctx = {"csrf_token": get_token(request), "error": None}
    if request.method == "POST":
        pwd = (request.POST.get("password") or "").strip()

        if not PASSWORD_REGEX.fullmatch(pwd):
            ctx["error"] = "密碼格式不正確：僅能使用英文或數字，且至少 6 碼。"
            return render(request, "ui/login.html", ctx)

        conf = SetupConfig.objects.first()
        if conf and pwd == conf.password_plain:
            request.session[SESSION_KEY] = True
            return redirect("dashboard")

        ctx["error"] = "密碼錯誤。"
    return render(request, "ui/login.html", ctx)

def logout_view(request: HttpRequest):
    """登出並回到 /login/"""
    request.session.pop(SESSION_KEY, None)
    return redirect("login")

# ====== 受保護頁面 ======
@_require_auth
def dashboard(request: HttpRequest):
    return render(request, "ui/dashboard.html")

@_require_auth
def devices(request: HttpRequest):
    return render(request, "ui/devices.html")

@_require_auth
def logs(request: HttpRequest):
    return render(request, "ui/logs.html")

@_require_auth
def settings_view(request: HttpRequest):
    return render(request, "ui/settings.html")

def healthz(request: HttpRequest):
    return HttpResponse("ok")

# ====== 資料庫概覽頁（含表頭） ======
@_require_auth
def db_overview(request: HttpRequest):
    rows = SetupConfig.objects.all().order_by("-id")
    return render(request, "ui/db_overview.html", {"rows": rows})

# --- Network ---
@_require_auth
def network_ip(request):
    ip_address = "N/A"
    netmask = "N/A"
    gateway = "N/A"
    dns = []

    try:
        # 取預設網路介面（通常是 eth0 或 wlan0）
        gws = netifaces.gateways()
        default_iface = gws['default'][netifaces.AF_INET][1] if 'default' in gws and netifaces.AF_INET in gws['default'] else None

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
    """回傳已綁定使用者清單，給前端表格輪詢用。"""
    qs = BoundUser.objects.all().order_by("-created_at").values(
        "id", "email", "user_name", "created_at", "last_seen"
    )
    items = list(qs)
    return JsonResponse({"items": items, "count": len(items)})
def api_network_ip(request: HttpRequest):
    """Return current network info for IP page polling."""
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
    """刪除已綁定的使用者"""
    try:
        user = get_object_or_404(BoundUser, id=user_id)
        user.delete()
        return JsonResponse({"status": "success"})
    except Exception as e:
        return JsonResponse({"status": "error", "message": str(e)}, status=400)

@_require_auth
def network_port(request):
    # TODO: 這裡之後接埠號設定
    return render(request, "ui/network_port.html", {"port": 8000})

# --- User ---
@_require_auth
def user_bound(request):
    # TODO: 這裡之後接資料庫查詢已綁定使用者
    bound_users = [
        {"id": 1, "name": "Alice", "created_at": "2025-09-20"},
        {"id": 2, "name": "Bob", "created_at": "2025-09-22"},
    ]
    return render(request, "ui/user_bound.html", {"users": bound_users})

# --- 裝置 ---
@_require_auth
def device_change_password(request):
    return render(request, "ui/device_change_password.html")

@_require_auth
def device_info(request):
    # TODO: 讀取你的裝置資訊（型號/序號/版本…）
    info = {"model": "Raspberry Pi", "version": "v1.0.0", "serial": "RPi-XXXX"}
    return render(request, "ui/device_info.html", {"info": info})

@_require_auth
@require_http_methods(["GET"])
def api_metrics(request: HttpRequest):
    """Return live system metrics for the dashboard (polled by front-end)."""
    cpu_percent = psutil.cpu_percent(interval=0.3)

    mem = psutil.virtual_memory()
    ram_used_mb  = round(mem.used / (1024**2))
    ram_total_mb = round(mem.total / (1024**2))
    ram_percent  = mem.percent

    disk = psutil.disk_usage("/")
    disk_used_gb  = round(disk.used / (1024**3), 1)
    disk_total_gb = round(disk.total / (1024**3), 1)
    disk_percent  = disk.percent

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
        "https": group({443, 8443}),            # 常見 HTTPS 埠
        "ssh": group({22}),                     # SSH
    }
    return JsonResponse(data)

@_require_auth
def user_bound(request):
    users = BoundUser.objects.all().order_by("-created_at")
    return render(request, "ui/user_bound.html", {"users": users})


@_require_auth
@require_http_methods(["GET", "POST"])
def device_change_password(request):
    errors = {}
    if request.method == "POST":
        old = (request.POST.get("old_password") or "").strip()
        new1 = (request.POST.get("new_password1") or "").strip()
        new2 = (request.POST.get("new_password2") or "").strip()

        cfg = SetupConfig.objects.first()

        # 後端驗證（與前端規則一致，且檢查舊密碼正確性）
        if not cfg:
            errors["old_password"] = "尚未設定初始密碼"
        elif old != cfg.password_plain:
            errors["old_password"] = "舊密碼錯誤"

        if not new1:
            errors["new_password1"] = "新密碼不可為空"
        elif len(new1) < 6:
            errors["new_password1"] = "新密碼至少 6 碼"
        elif not new1.isalnum():  # 僅英數
            errors["new_password1"] = "僅限英數（不可空白、符號、中文）"

        if not new2:
            errors["new_password2"] = "請再次輸入新密碼"
        elif new1 and new1 != new2:
            errors["new_password2"] = "兩次輸入的新密碼不一致"

        if not errors:
            # 1) 更新密碼
            cfg.password_plain = new1
            cfg.save(update_fields=["password_plain"])
            # 2) 強制登出（清 session）
            request.session.flush()
            # 3) 導回登入頁
            return redirect("login")

    # GET 或驗證失敗
    return render(request, "ui/device_change_password.html", {"errors": errors})

@_require_auth
def device_info(request):
    # TODO: 之後接實際的裝置資訊（序號/版本/狀態）
    info = {
        "serial": "預留",       # 序號
        "version": "預留",      # 版本
        "status": 1,           # 1=已連線, 0=未連線
    }
    return render(request, "ui/device_info.html", {"info": info})
# ====== API（啟用；登入保護） ======
@_require_auth
@require_http_methods(["GET"])
def api_setupconfig_list(request: HttpRequest):
    """
    回傳 SetupConfig 清單（示範用）
    正式上線請視需要再加白名單 / Token。
    """
    data = list(
        SetupConfig.objects.all()
        .order_by("-id")
        .values("id", "password_plain", "created_at")
    )
    return JsonResponse({"items": data, "count": len(data)})