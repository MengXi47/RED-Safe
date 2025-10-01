# ui/views.py
import re
import psutil
import subprocess
import netifaces
import socket
import uuid
import qrcode
import io
import base64
from django.shortcuts import render, redirect
from django.http import HttpRequest, HttpResponse, JsonResponse
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_http_methods
from django.middleware.csrf import get_token
from .models import SetupConfig, BoundUser, PgConfig
from django.shortcuts import get_object_or_404



SESSION_KEY = "setup_authed"

# 密碼規則：僅英數、至少 6 碼
PASSWORD_REGEX = re.compile(r'^[A-Za-z0-9]{6,}$')

def _edge_id() -> str:
    """產生當前設備的 edge_id（優先用 MAC，否則用 hostname）。"""
    try:
        mac = uuid.getnode()
        # 若最高位不是本機隨機位，視為真實 MAC
        if (mac >> 40) % 2 == 0:
            return f"edge-{mac:012x}"
    except Exception:
        pass
    return f"edge-{socket.gethostname()}"

def _is_configured() -> bool:
    """優先以 PostgreSQL 的 PgConfig 判斷是否已設定；若查詢失敗則回退到 SetupConfig。"""
    try:
        eid = _edge_id()
        if PgConfig.objects.filter(edge_id=eid).exists():
            return True
    except Exception:
        pass
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

        # 寫入 PostgreSQL 的 config 表（若已存在該 edge，則更新密碼）
        eid = _edge_id()
        row = PgConfig.objects.filter(edge_id=eid).first()
        if row:
            PgConfig.objects.filter(edge_id=eid).update(edge_password=pwd)
        else:
            PgConfig.objects.create(edge_id=eid, edge_password=pwd)
        request.session[SESSION_KEY] = True
        return redirect("dashboard")

    return render(request, "ui/setup.html", ctx)

# ====== 登入（之後每次都走這裡） ======
@require_http_methods(["GET", "POST"])
def login_view(request: HttpRequest):
    # 若改成永遠走 PG，可把 _is_configured() 這段邏輯簡化/移除
    ctx = {"csrf_token": get_token(request), "error": None}

    if request.method == "POST":
        pwd = (request.POST.get("password") or "").strip()

        # （沿用你之前的格式檢查）
        if not PASSWORD_REGEX.fullmatch(pwd):
            ctx["error"] = "密碼格式不正確：僅能使用英文或數字，且至少 6 碼。"
            return render(request, "ui/login.html", ctx)

        # ---- 核心：改用 PostgreSQL 的 config 表 ----
        try:
            # 以本機 edge_id 取得這台設備的設定
            eid = _edge_id()
            row = PgConfig.objects.filter(edge_id=eid).first()
            if row and pwd == (row.edge_password or ""):
                request.session[SESSION_KEY] = True
                return redirect("dashboard")
            else:
                ctx["error"] = "密碼錯誤。"
        except Exception as e:
            ctx["error"] = f"資料庫連線/查詢失敗：{e}"

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

@_require_auth
@require_http_methods(["GET"])
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

# --- 裝置 ---
@_require_auth
def device_change_password(request):
    return render(request, "ui/device_change_password.html")

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

        # 以本機 edge_id 定位這台設備的設定
        eid = _edge_id()
        row = PgConfig.objects.filter(edge_id=eid).first()

        if not row:
            errors["old_password"] = "尚未於資料庫建立此設備的 config 記錄"
        elif old != (row.edge_password or ""):
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
            # 寫回 PG（這台設備）
            PgConfig.objects.filter(edge_id=eid).update(edge_password=new1)

            # 強制登出，安全起見
            request.session.flush()
            return redirect("login")

    return render(request, "ui/device_change_password.html", {"errors": errors})

# 替換現有 device_info view 為下面版本
@_require_auth
def device_info(request):
    # 若未接實際裝置資料，先使用預設（題主要求）
    serial = "12345678"    # 預留：序號（目前固定為 12345678）
    version = "v1.0.0"     # 預留：版本
    status = 1             # 1 = 已連線, 0 = 未連線
    password = "12345678"  # 預留密碼（題主要求預設為 12345678）

    # QR code 內容格式（你可以按需要改格式）
    # 我用簡單 key=value;key=value 讓掃描端能解析
    payload = f"serial={serial};password={password}"

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
    }
    return render(request, "ui/device_info.html", {"info": info, "qrcode_data": qrcode_data})


# ====== QR Image Endpoint ======
@_require_auth
@require_http_methods(["GET"])
def device_qr(request: HttpRequest):
    serial = "12345678"
    password = "12345678"
    payload = f"serial={serial};password={password}"

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