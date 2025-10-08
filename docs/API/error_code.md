## 錯誤碼

| 錯誤碼 | 說明 |
|--------|------|
| 0 | 成功 |
| 120 | Invalid Edge Id format |
| 121 | Edge version must follow format X.Y.Z (e.g., 1.0.0) |
| 122 | edge_name為空白 |
| 123 | edge_id不存在 |
| 124 | email格式錯誤 |
| 125 | edge_id為空 |
| 126 | access_token失效 |
| 127 | 缺少access_token |
| 128 | 帳號密碼錯誤 |
| 129 | Email為空 |
| 130 | Password為空 |
| 131 | user_name為空 |
| 132 | refresh_token失效 |
| 133 | Email已存在 |
| 134 | user_id&edge_id已綁定 |
| 135 | user_id&edge_id未綁定 |
| 136 | edge_id 數量為 0 |
| 137 | refresh_token為空 |
| 138 | user_name為空 |
| 139 | user_name過長 |
| 140 | edge_name過長 |
| 141 | new_password為空 |
| 142 | user不存在 |
| 143 | 舊密碼錯誤 |
| 144 | edge_password為空 |
| 145 | edge version為空 |
| 146 | edge_id已存在 |
| 147 | edge_password錯誤 |
| 148 | new_edge_password為空 |
| 150 | 需要二階段驗證 OTP |
| 151 | OTP 未啟用 |
| 152 | OTP 驗證失敗 |

---

## iOS 服務錯誤識別碼

| 識別碼 | HTTP 狀態 | 說明 |
|--------|-----------|------|
| `MISSING_AUTHORIZATION_HEADER` | 401 | 未提供 `Authorization` header |
| `INVALID_AUTHORIZATION_HEADER` | 401 | Header 格式錯誤或缺少 Bearer Token |
| `INVALID_TOKEN` | 401 | JWT 驗證失敗 |
| `IOS_DEVICE_NOT_FOUND` | 404 | 指定的裝置不屬於目前使用者 |
| `APNS_TOKEN_IN_USE` | 409 | 其他裝置已使用該 APNS Token |
| `USER_SERVICE_UNAVAILABLE` | 503 | 呼叫 user 服務進行 JWT 驗證時失敗 |
