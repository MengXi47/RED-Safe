# 📱 iOS 服務 API 文件

iOS 服務用來註冊與更新行動裝置的 APNS Token，並需透過使用者服務的 JWT 驗證。

---

## POST /ios/reg
註冊新的 iOS 裝置或更新既有裝置的 APNS Token。

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
```json
{
  "ios_device_id": "d9eb8c22-7dba-4b20-a0ef-5fb6f6b02b4e",
  "apns_token": "<APNS Token>",
  "device_name": "iPhone 15 Pro"
}
```

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `ios_device_id` | UUID | ❌ | 指定時會更新該裝置，未指定則建立新裝置 |
| `apns_token` | string | ✅ | 會自動去除首尾空白，最長 512 字元 |
| `device_name` | string | ❌ | 會自動去除首尾空白，最長 255 字元 |

### 成功回應 (200)
```json
{
  "ios_device_id": "d9eb8c22-7dba-4b20-a0ef-5fb6f6b02b4e",
  "apns_token": "<APNS Token>",
  "device_name": "iPhone 15 Pro"
}
```

### 失敗回應範例
```json
{
  "error_code": "INVALID_TOKEN"
}
```

### 常見錯誤碼
- `130` apns_token 為空
- `131` apns_token 長度超過 512
- `132` device_name 長度超過 255
- `MISSING_AUTHORIZATION_HEADER` 未提供 `Authorization` header
- `INVALID_AUTHORIZATION_HEADER` Header 格式錯誤或 Bearer Token 為空
- `INVALID_TOKEN` JWT 驗證失敗或 user id 無效
- `IOS_DEVICE_NOT_FOUND` 指定裝置不屬於目前使用者
- `APNS_TOKEN_IN_USE` apns_token 已被其他裝置使用
- `USER_SERVICE_UNAVAILABLE` 呼叫 user 服務驗證 JWT 失敗
