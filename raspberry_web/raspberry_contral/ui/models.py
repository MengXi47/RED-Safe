from django.db import models

class SetupConfig(models.Model):
    # 明碼存放
    password_plain = models.CharField(max_length=128, default="RedSafe123")
    created_at = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f"{self.id}: {self.password_plain}"

class BoundUser(models.Model):
    email = models.EmailField(unique=True, db_index=True, verbose_name="Email")
    user_name = models.CharField(max_length=100, verbose_name="使用者名稱")
    created_at = models.DateTimeField(auto_now_add=True, verbose_name="綁定時間")
    last_seen = models.DateTimeField(null=True, blank=True, verbose_name="上次上線")

    class Meta:
        ordering = ["-created_at"]
        verbose_name = "綁定使用者"
        verbose_name_plural = "綁定使用者"

    def __str__(self):
        return f"{self.user_name} <{self.email}>"