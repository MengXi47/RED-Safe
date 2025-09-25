from django.urls import path
from . import views

urlpatterns = [
    path("", views.dashboard, name="dashboard"),
    path("devices/", views.devices, name="devices"),
    path("logs/", views.logs, name="logs"),
    path("settings/", views.settings_view, name="settings"),
]