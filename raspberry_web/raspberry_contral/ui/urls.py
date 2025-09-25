from django.urls import path
from . import views

urlpatterns = [
    path("setup/", views.setup_view, name="setup"),     # 首次設定
    path("login/", views.login_view, name="login"),     # 後續登入
    path("logout/", views.logout_view, name="logout"),  # 登出

    path("", views.dashboard, name="dashboard"),
    path("devices/", views.devices, name="devices"),
    path("logs/", views.logs, name="logs"),
    path("settings/", views.settings_view, name="settings"),
    path("healthz/", views.healthz, name="healthz"),
    path("db/", views.db_overview, name="db_overview"),
    # path("api/setupconfig/", views.api_setupconfig_list, name="api_setupconfig_list"),
]