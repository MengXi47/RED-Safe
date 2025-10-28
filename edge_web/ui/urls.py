from django.urls import path
from . import views

urlpatterns = [
    # 既有
    path("setup/", views.setup_view, name="setup"),
    path("login/", views.login_view, name="login"),
    path("logout/", views.logout_view, name="logout"),
    path("", views.dashboard, name="dashboard"),
    path("logs/", views.logs, name="logs"),
    path("settings/", views.settings_view, name="settings"),
    path("db/", views.db_overview, name="db_overview"),
    path("api/setupconfig/", views.api_setupconfig_list, name="api_setupconfig_list"),
    path("api/system/logs", views.api_system_logs, name="api_system_logs"),

    # 新增：Network
    path("network/ip/", views.network_ip, name="network_ip"),
    path("network/port/", views.network_port, name="network_port"),

    # 新增：User
    path("user/bound/", views.user_bound, name="user_bound"),
    path("api/user/bound/", views.api_user_bound, name="api_user_bound"),
    path("api/user/remove/", views.api_user_remove, name="api_user_remove"),
    path("device/device_change-password/", views.device_change_password, name="device_change_password"),
    path("device/change-password/", views.device_change_password),

    # 新增：裝置
    path("device/info/", views.device_info, name="device_info"),
    path("api/device/info/", views.api_device_info, name="api_device_info"),
    path("api/metrics/", views.api_metrics, name="api_metrics"),
    path("network/ip/", views.network_ip, name="network_ip"),
    path("api/network/port/", views.api_network_port, name="api_network_port"),
    path("device/qrcode.png", views.device_qr, name="device_qr"),
    path("cameras", views.cameras, name="cameras"),
    path("api/cameras/scan", views.api_cameras_scan, name="api_cameras_scan"),
    path("api/cameras/bind", views.api_cameras_bind, name="api_cameras_bind"),
    path("api/cameras/unbind", views.api_cameras_unbind, name="api_cameras_unbind"),
    path("api/cameras/bound", views.api_cameras_bound_list, name="api_cameras_bound"),
    path("api/cameras/preview/probe", views.api_cameras_preview_probe, name="api_cameras_preview_probe"),
    path("api/cameras/preview/webrtc/offer", views.api_cameras_preview_webrtc_offer, name="api_cameras_preview_webrtc_offer"),
    path("api/cameras/preview/webrtc/hangup", views.api_cameras_preview_webrtc_hangup, name="api_cameras_preview_webrtc_hangup"),
]
