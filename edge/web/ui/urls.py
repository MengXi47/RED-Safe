from django.urls import path
from . import views

urlpatterns = [
    # 既有
    path("setup/", views.setup_view, name="setup"),
    path("login/", views.login_view, name="login"),
    path("logout/", views.logout_view, name="logout"),
    path("", views.dashboard, name="dashboard"),
    path("logs/", views.logs, name="logs"),
    path("events/", views.events, name="events"),
    path("settings/", views.settings_view, name="settings"),
    path("db/", views.db_overview, name="db_overview"),
    path("api/setupconfig/", views.api_setupconfig_list, name="api_setupconfig_list"),
    path("api/system/logs", views.api_system_logs, name="api_system_logs"),
    path("api/events/", views.api_events, name="api_events"),

    # 新增：Network
    path("network/config/", views.network_config, name="network_config"),
    # 舊路徑相容處理
    path("network/ip/", views.network_config, name="network_ip"),
    path("network/port/", views.network_config, name="network_port"),

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
    path("api/network/port/", views.api_network_port, name="api_network_port"),
    path("api/network/ip/", views.api_network_ip, name="api_network_ip"),
    path("api/network/config/apply/", views.api_network_config_apply, name="api_network_config_apply"),
    path("device/qrcode.png", views.device_qr, name="device_qr"),
    path("cameras", views.cameras, name="cameras"),
    path("cameras/live", views.cameras, name="cameras_live"),
    path("cameras/detection", views.cameras, name="cameras_detection"),
    path("api/cameras/scan", views.api_cameras_scan, name="api_cameras_scan"),
    path("api/cameras/bind", views.api_cameras_bind, name="api_cameras_bind"),
    path("api/cameras/unbind", views.api_cameras_unbind, name="api_cameras_unbind"),
    path("api/cameras/bound", views.api_cameras_bound_list, name="api_cameras_bound"),
    path("api/cameras/policies", views.api_cameras_policies, name="api_cameras_policies"),
    path("api/cameras/policies/fall", views.api_camera_policy_fall, name="api_camera_policy_fall"),
    path("api/cameras/policies/inactivity", views.api_camera_policy_inactivity, name="api_camera_policy_inactivity"),
    path("api/cameras/bed_roi", views.api_camera_bed_roi, name="api_camera_bed_roi"),
    path("api/cameras/bed_roi/snapshot", views.api_bed_roi_snapshot, name="api_bed_roi_snapshot"),
    path("api/cameras/preview/probe", views.api_cameras_preview_probe, name="api_cameras_preview_probe"),
    path("api/cameras/preview/webrtc/offer", views.api_cameras_preview_webrtc_offer, name="api_cameras_preview_webrtc_offer"),
    path("api/cameras/preview/webrtc/hangup", views.api_cameras_preview_webrtc_hangup, name="api_cameras_preview_webrtc_hangup"),
]
