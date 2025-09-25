# ui/views.py
import re
from django.shortcuts import render, redirect
from django.http import HttpRequest, HttpResponse, JsonResponse
from django.views.decorators.http import require_http_methods
from django.middleware.csrf import get_token
from .models import SetupConfig

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

# ====== API（啟用；登入保護） ======
# @_require_auth
# @require_http_methods(["GET"])
# def api_setupconfig_list(request: HttpRequest):
#     """
#     回傳 SetupConfig 清單（示範用）
#     正式上線請視需要再加白名單 / Token。
#     """
#     data = list(
#         SetupConfig.objects.all()
#         .order_by("-id")
#         .values("id", "password_plain", "created_at")
#     )
#     return JsonResponse({"items": data, "count": len(data)})