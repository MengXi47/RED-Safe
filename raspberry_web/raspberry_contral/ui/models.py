from django.db import models


class EdgeConfig(models.Model):
    """對應 PostgreSQL 中的 config 表，僅存放 edge 資訊。"""

    edge_id = models.TextField(primary_key=True)
    edge_password = models.TextField(blank=True, null=True)

    class Meta:
        db_table = "config"
        managed = False

from django.db import models

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