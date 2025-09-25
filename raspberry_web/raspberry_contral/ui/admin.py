from django.contrib import admin
from .models import SetupConfig

@admin.register(SetupConfig)
class SetupConfigAdmin(admin.ModelAdmin):
    list_display = ("id", "password_plain", "created_at")
    ordering = ("-id",)
    search_fields = ("id", "password_plain")