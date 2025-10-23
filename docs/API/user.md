# 👤 User 服務 API 文件

除特別註明外，所有端點都需要在 `Authorization` header 中附上 `Bearer {access_token}`，並使用 JSON 交換資料。

---

## GET /user/info
取得目前使用者的基本資料與二階段驗證狀態。

### Headers
- `Authorization: Bearer {access_token}`

### 成功回應 (200)
```json
{
  "user_name": "USER_NAME",
  "email": "user@example.com",
  "otp_enabled": true
}
```

### 常見錯誤碼
- `126` access_token 失效
- `127` 缺少 access_token
- `142` user 不存在

---

## POST /user/bind
將目前使用者與指定 Edge 裝置綁定。

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "edge_name": "客廳",
  "edge_password": "EdgeSecret"
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
- `122` edge_name 為空
- `125` Edge Id 為空
- `126` access_token 失效
- `127` 缺少 access_token
- `134` 已綁定
- `144` Edge 密碼為空
- `147` Edge 密碼錯誤

---

## POST /user/unbind/{edge_id}
解除與指定 Edge 裝置的綁定。

### Headers
- `Authorization: Bearer {access_token}`

### Path 參數
| 參數 | 說明 |
|------|------|
| `edge_id` | 要解除綁定的 Edge 裝置 ID |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `120` Edge Id 格式錯誤
- `123` Edge 不存在
- `125` Edge Id 為空
- `126` access_token 失效
- `127` 缺少 access_token
- `135` 尚未綁定

---

## GET /user/list/edge_id
列出目前使用者已綁定的 Edge 裝置。

### Headers
- `Authorization: Bearer {access_token}`

### 成功回應 (200)
```json
{
  "edges": [
    {
      "edge_id": "RED-AAAAAAAA",
      "display_name": "客廳",
      "is_online": true
    }
  ]
}
```

- `is_online` 由 Redis 中的心跳狀態推論。

### 常見錯誤碼
- `126` access_token 失效
- `127` 缺少 access_token
- `136` 尚未綁定任何 Edge

---

## POST /user/update/edge_name
更新已綁定 Edge 裝置的顯示名稱。

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "edge_name": "臥室"
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
- `122` edge_name 為空
- `125` Edge Id 為空
- `126` access_token 失效
- `127` 缺少 access_token
- `135` 尚未綁定
- `140` edge_name 過長

---

## POST /user/update/user_name
更新使用者顯示名稱。

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
```json
{
  "user_name": "新的暱稱"
}
```

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `126` access_token 失效
- `127` 缺少 access_token
- `138` user_name 為空
- `139` user_name 過長
- `142` user 不存在

---

## POST /user/update/password
更新使用者登入密碼。

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
```json
{
  "password": "OldPassword",
  "new_password": "NewPassword"
}
```

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `126` access_token 失效
- `127` 缺少 access_token
- `130` Password 為空
- `141` new_password 為空
- `142` user 不存在
- `143` 舊密碼錯誤
- `AUTH_GRPC_FAILED` 呼叫 auth gRPC 失敗

---

## POST /user/update/edge_password
更新使用者已綁定 Edge 的連線密碼。服務會透過 gRPC 呼叫 Edge 服務。

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

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
- `126` access_token 失效
- `127` 缺少 access_token
- `135` 尚未綁定
- `144` Edge 密碼為空
- `148` 新 Edge 密碼為空
- `123`、`147` 由 Edge 服務回傳
- `AUTH_GRPC_FAILED` 呼叫 auth gRPC 失敗

---

## POST /user/edge/command
對綁定中的 Edge 裝置下發控制指令。服務會在 Redis 保存請求並透過 MQTT 發佈。

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "code": "REBOOT",
  "payload": {
    "delay": 5
  }
}
```

### 成功回應 (200)
```json
{
  "trace_id": "5edbc74e-24b7-4749-8bb7-3e32a47d9ef4"
}
```

### 常見錯誤碼
- `120`、`125` Edge Id 驗證失敗
- `126` access_token 失效
- `127` 缺少 access_token
- `135` Edge 未綁定
- `149` 缺少 code
- `MQTT_SUBSCRIBE_FAILED` 無法建立 MQTT 訂閱
- `MQTT_PUBLISH_FAILED` MQTT 發佈失敗

---

## GET /user/sse/get/command/{trace_id}
以 Server-Sent Events 方式取得 Edge 指令的回傳結果。建議在指令下發後立即建立連線。

### Headers
- `Authorization: Bearer {access_token}`
- `Accept: text/event-stream`

### Path 參數
| 參數 | 說明 |
|------|------|
| `trace_id` | 下發指令時取得的 `trace_id` |

### 成功回應
- HTTP 200，`Content-Type: text/event-stream`
- 事件名稱固定為 `command`

範例事件：
```
id: 5edbc74e-24b7-4749-8bb7-3e32a47d9ef4
event: command
data: {"status":"ok","payload":{"result":"done"}}
```

若 Edge 未回應，會傳回 `data: "notfound"` 後結束連線。

### 常見錯誤碼
- `126` access_token 失效
- `127` 缺少 access_token
- `135` Edge 未綁定
- `149` trace_id 格式錯誤
- `155` 控制指令不存在

---

## GET /user/userid
提供後端或其他服務依 Email 查詢 user id。此端點 **不** 需要 Authorization。

### Query 參數
| 參數 | 必填 | 說明 |
|------|------|------|
| `email` | ✅ | 目標使用者的 Email |

### 成功回應 (200)
```json
{
  "user_id": "85f22dda-efc2-459d-b518-640400a69e8d"
}
```

### 常見錯誤碼
- `129` Email 為空
- `EMAILNOTFOUND` 查無該 Email
