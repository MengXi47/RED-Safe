# 🖥️ Edge 服務 API 文件

Edge 服務提供邊緣裝置的註冊、密碼維護、上線回報以及使用者綁定查詢。

所有端點皆回傳 JSON。

---

## POST /edge/reg
註冊全新的 Edge 裝置並設定初始密碼。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "edge_password": "EdgeSecret",
  "version": "1.0.0"
}
```

| 欄位 | 型別 | 必填 | 說明 |
|------|------|------|------|
| `edge_id` | string | ✅ | 格式需為 `RED-XXXXXXXX` (大寫十六進位) |
| `edge_password` | string | ✅ | 初始密碼，伺服器會雜湊儲存 |
| `version` | string | ✅ | 版本格式 `X.Y.Z` |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `120` Edge Id 格式錯誤
- `125` Edge Id 為空
- `144` Edge 密碼為空
- `145` Edge 版本為空
- `121` 版本格式不符
- `146` Edge Id 已存在

---

## POST /edge/update/edge_password
由 Edge 裝置直接更新自身密碼。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "edge_password": "CurrentEdgeSecret",
  "new_edge_password": "NextEdgeSecret"
}
```

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `120` Edge Id 格式錯誤
- `125` Edge Id 為空
- `144` 現行密碼為空
- `148` 新密碼為空
- `123` Edge 不存在
- `147` Edge 密碼錯誤

---

## POST /edge/online
Edge 裝置上線時回報狀態，服務會更新最後在線時間並註冊心跳。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "started_at": "2025-01-15T08:30:00+08:00"
}
```

| 欄位 | 型別 | 必填 | 說明 |
|------|------|------|------|
| `edge_id` | string | ✅ | Edge 裝置 ID |
| `started_at` | string | ❌ | ISO 8601 時間戳記，僅供紀錄 |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `120` Edge Id 格式錯誤
- `125` Edge Id 為空
- `123` Edge 不存在

---

## GET /edge/user/list
查詢指定 Edge 已綁定的使用者列表。

### Query 參數

| 參數 | 必填 | 說明 |
|------|------|------|
| `edge_id` | ✅ | Edge 裝置 ID |

### 成功回應 (200)
```json
{
  "users": [
    {
      "user_id": "0e1f9f6f-55aa-4e17-a4d0-9f89aa31d0c5",
      "email": "user@example.com",
      "user_name": "USER_NAME",
      "bind_at": "2025-01-01T12:00:00+08:00",
      "last_online": "2025-01-15T09:00:00+08:00"
    }
  ]
}
```

- `last_online` 欄位為裝置最後回報時間；若 gRPC 回傳空字串會自動帶入 Edge 端保存的最後在線時間。

### 常見錯誤碼
- `120` Edge Id 格式錯誤
- `125` Edge Id 為空
- `123` Edge 不存在
- `SERVICE_UNAVAILABLE` 轉呼叫 user 服務失敗

---

## POST /edge/user/unbind
解除指定使用者與 Edge 的綁定。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "email": "user@example.com"
}
```

- 目前服務以 Email 辨識使用者。

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `120` Edge Id 格式錯誤
- `125` Edge Id 為空
- `123` Edge 不存在
- `127` 缺少必填欄位 (email)
- `USER_SERVICE_UNAVAILABLE` gRPC 呼叫 user 服務失敗
