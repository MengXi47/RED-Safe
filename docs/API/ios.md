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
| `apns_token` | string | ✅ | 最長 512 字元，不能為空 |
| `device_name` | string | ❌ | 最長 255 字元 |

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

### 失敗情境
| HTTP 狀態 | `error_code` | 說明 |
|-----------|--------------|------|
| 401 | `MISSING_AUTHORIZATION_HEADER` | 未提供 `Authorization` header |
| 401 | `INVALID_AUTHORIZATION_HEADER` | Header 格式錯誤或 Bearer Token 為空 |
| 401 | `INVALID_TOKEN` | JWT 驗證失敗或 user id 無效 |
| 404 | `IOS_DEVICE_NOT_FOUND` | 指定的 `ios_device_id` 不屬於目前使用者 |
| 409 | `APNS_TOKEN_IN_USE` | 提供的 `apns_token` 已綁定其他裝置 |
| 503 | `USER_SERVICE_UNAVAILABLE` | 無法透過 gRPC 驗證 JWT，請稍後重試 |
