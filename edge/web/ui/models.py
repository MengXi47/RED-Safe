from django.db import models


class EdgeConfig(models.Model):
    """對應 PostgreSQL 中的 config 表，僅存放 edge 資訊。"""

    edge_id = models.TextField(primary_key=True)
    edge_password = models.TextField(blank=True, null=True)

    class Meta:
        db_table = "config"
        managed = False

class ConnectedIPC(models.Model):
    """
    對應本地 PostgreSQL 的 connected_ipc 表。
    注意：資料表本身沒有主鍵，但為了讓 Django ORM 正常運作，
    在模型層把 mac_address 視為主鍵。因為 managed=False，資料庫不會被更動。
    """
    mac_address = models.CharField(max_length=255, primary_key=True)
    ip_address = models.CharField(max_length=255)
    ipc_name = models.CharField(max_length=255, blank=True, null=True)
    custom_name = models.CharField(max_length=255, blank=True, null=True)
    ipc_account = models.CharField(max_length=255, blank=True, null=True)
    ipc_password = models.CharField(max_length=255, blank=True, null=True)

    class Meta:
        db_table = "connected_ipc"
        managed = False


class IpcFallDetectionPolicy(models.Model):
    """對應 ipc_fall_detection_policy 表，存放單一攝影機的跌倒偵測開關。"""

    ip_address = models.GenericIPAddressField(primary_key=True)
    enabled = models.BooleanField(default=False)
    updated_at = models.DateTimeField()

    class Meta:
        db_table = "ipc_fall_detection_policy"
        managed = False


class IpcInactivityPolicy(models.Model):
    """對應 ipc_inactivity_policy 表，存放長時間靜止/未活動設定。"""

    ip_address = models.GenericIPAddressField(primary_key=True)
    enabled = models.BooleanField(default=True)
    idle_minutes = models.IntegerField(default=5)
    quiet_start = models.TimeField(null=True)
    quiet_end = models.TimeField(null=True)
    quiet_enabled = models.BooleanField(default=False)
    updated_at = models.DateTimeField()

    class Meta:
        db_table = "ipc_inactivity_policy"
        managed = False


class IpcBedRoiZone(models.Model):
    """對應 ipc_bed_roi_zones，存放夜間離床用的床區 ROI。"""

    ip_address = models.GenericIPAddressField(primary_key=True)
    points = models.JSONField()
    enabled = models.BooleanField(default=True)
    quiet_start = models.TimeField(null=True)
    quiet_end = models.TimeField(null=True)
    updated_at = models.DateTimeField()

    class Meta:
        db_table = "ipc_bed_roi_zones"
        managed = False


class EdgeEvent(models.Model):
    """對應 edge_events，記錄偵測事件（跌倒、離床等）。"""

    id = models.BigAutoField(primary_key=True)
    occurred_at = models.DateTimeField()
    event_type = models.TextField()
    payload = models.JSONField(null=True)

    class Meta:
        db_table = "edge_events"
        managed = False
