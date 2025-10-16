"""主要 URL 設定：僅匯入 UI 應用的路由。"""

from django.urls import include, path

urlpatterns = [
    path("", include("ui.urls")),
]