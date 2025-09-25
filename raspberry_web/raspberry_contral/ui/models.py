from django.db import models

class SetupConfig(models.Model):
    # 明碼存放
    password_plain = models.CharField(max_length=128, default="RedSafe123")
    created_at = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f"{self.id}: {self.password_plain}"