from django.contrib import admin
from .models import SetupConfig, BoundUser

@admin.register(SetupConfig)
class SetupConfigAdmin(admin.ModelAdmin):
    list_display = ("id", "password_plain", "created_at")
    ordering = ("-id",)
    search_fields = ("id", "password_plain")

@admin.register(BoundUser)
class BoundUserAdmin(admin.ModelAdmin):
    list_display = ("id", "email", "user_name", "created_at", "last_seen")
    search_fields = ("email", "user_name")
    list_filter = ("created_at",)
    ordering = ("-created_at",)