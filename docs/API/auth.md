# 🔐 Auth API 文件

Auth 服務提供註冊、登入與刷新 Access Token 的功能。

---

## POST /auth/signup
建立全新的使用者帳號。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "email": "user@example.com",
  "user_name": "USER_NAME",
  "password": "StrongPassword123"
}
```

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `email` | string | ✅ | 合法 Email 格式，不能為空 |
| `user_name` | string | ✅ | 最長 16 字元，不能為空 |
| `password` | string | ✅ | 不能為空 |

### 成功回應 (200)
```json
{
  "user_id": "85f22dda-efc2-459d-b518-640400a69e8d",
  "user_name": "displayName"
}
```

### 失敗回應範例
```json
{
  "error_code": "133"
}
```

**常見錯誤碼**：`124`、`129`、`130`、`131`、`139`、`133`

---

## POST /auth/signin
使用 Email 與密碼換取 JWT 與 Refresh Token。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "email": "user@example.com",
  "password": "Password"
}
```

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `email` | string | ✅ | 合法 Email 格式，不能為空 |
| `password` | string | ✅ | 不能為空 |

### 成功回應 (200)
```json
{
  "user_name": "USER_NAME",
  "access_token": "eyJhbGciOiJIUzI1NiJ9...",
  "refresh_token": "ajh23kjasd..."
}
```

### 失敗回應範例
```json
{
  "error_code": "128"
}
```

**常見錯誤碼**：`124`、`128`、`129`、`130`

---

## POST /auth/refresh
透過 Refresh Token 換取新的 Access Token。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "refresh_token": "rFT4nq9J..."
}
```

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `refresh_token` | string | ✅ | 不能為空 |

### 成功回應 (200)
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiJ9..."
}
```

### 失敗回應範例
```json
{
  "error_code": "132"
}
```

**常見錯誤碼**：`132`、`137`
