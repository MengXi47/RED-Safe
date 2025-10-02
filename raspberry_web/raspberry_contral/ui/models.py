from django.db import models


class EdgeConfig(models.Model):
    """對應 PostgreSQL 中的 config 表，僅存放 edge 資訊。"""

    edge_id = models.TextField(primary_key=True)
    edge_password = models.TextField(blank=True, null=True)

    class Meta:
        db_table = "config"
        managed = False
