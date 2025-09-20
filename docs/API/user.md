# 👤 User API 文件

所有 User 服務的 HTTP 端點皆需要 JWT Access Token。請在 `Authorization` header 中附上 `Bearer {access_token}`。

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

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `edge_id` | string | ✅ | 格式 `RED-XXXXXXXX`，大寫十六進位 |
| `edge_name` | string | ✅ | 最長 16 字元，不能為空 |
| `edge_password` | string | ✅ | 不能為空 |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 失敗回應範例
```json
{
  "error_code": "147"
}
```

**常見錯誤碼**：`120`、`122`、`123`、`125`、`126`、`127`、`134`、`144`、`147`

---

## POST /user/unbind/{edge_id}
解除指定 Edge 裝置的綁定。

### Headers
- `Authorization: Bearer {access_token}`

### 路徑參數
| 參數 | 說明 |
|------|------|
| `edge_id` | 要解除綁定的 Edge 裝置 ID |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```


### 失敗回應範例
```json
{
  "error_code": "135"
}
```

**常見錯誤碼**：`123`、`125`、`126`、`127`、`135`

---

## GET /user/list/edge_id
列出目前帳號已綁定的 Edge 裝置。

### Headers
- `Authorization: Bearer {access_token}`

### 成功回應 (200)
```json
{
  "edges": [
    {
      "edge_id": "RED-AAAAAAAA",
      "display_name": "客廳"
    },
    {
      "edge_id": "RED-BBBBBBBB",
      "display_name": "廚房"
    }
  ]
}
```


### 失敗回應範例
```json
{
  "error_code": "126"
}
```

**常見錯誤碼**：`126`、`127`、`136`

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

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `edge_id` | string | ✅ | 格式 `RED-XXXXXXXX` |
| `edge_name` | string | ✅ | 最長 16 字元，不能為空 |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```


### 失敗回應範例
```json
{
  "error_code": "135"
}
```

**常見錯誤碼**：`120`、`122`、`125`、`126`、`127`、`135`、`140`

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

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `user_name` | string | ✅ | 最長 16 字元，不能為空 |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```


### 失敗回應範例
```json
{
  "error_code": "138"
}
```

**常見錯誤碼**：`126`、`127`、`138`、`139`、`142`

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

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `password` | string | ✅ | 不能為空 |
| `new_password` | string | ✅ | 不能為空 |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```


### 失敗回應範例
```json
{
  "error_code": "143"
}
```

**常見錯誤碼**：`126`、`127`、`130`、`141`、`142`、`143`

---

## POST /user/update/edge_password
更新已綁定 Edge 裝置的連線密碼。

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

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `edge_id` | string | ✅ | 格式 `RED-XXXXXXXX` |
| `edge_password` | string | ✅ | 不能為空 |
| `new_edge_password` | string | ✅ | 不能為空 |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```


### 失敗回應範例
```json
{
  "error_code": "147"
}
```

**常見錯誤碼**：`120`、`123`、`125`、`126`、`127`、`135`、`144`、`147`、`148`
