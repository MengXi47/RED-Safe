from django.contrib import admin

from .models import EdgeConfig


@admin.register(EdgeConfig)
class EdgeConfigAdmin(admin.ModelAdmin):
    list_display = ("edge_id", "edge_password")
    search_fields = ("edge_id",)
