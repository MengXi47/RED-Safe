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

### 啟用 OTP 時的回應 (200)
```json
{
  "error_code": "150"
}
```
帳號若已啟用二階段驗證，會回傳 `error_code = 150`，此時必須改呼叫「/auth/signin/otp」。

### 失敗回應範例
```json
{
  "error_code": "128"
}
```

**常見錯誤碼**：`124`、`128`、`129`、`130`、`150`

---

## POST /auth/create/otp
產生 TOTP 秘鑰並啟用二階段驗證，需提供已登入的 `access_token`。

### Headers
- `Authorization: Bearer <access_token>`

### Request Body
無

### 成功回應 (200)
```json
{
  "otp_key": "JBSWY3DPEHPK3PXPJBSWY3DP",
  "backup_codes": [
    "123456",
    "654321",
    "112233"
  ]
}
```
回傳的 `otp_key` 可手動輸入或轉換成 QR Code 匯入 Authenticator App。`backup_codes` 為一次性備援碼，使用後會作廢。

### 失敗回應範例
```json
{
  "error_code": "153"
}
```

**常見錯誤碼**：`126`（token 失效）、`142`（使用者不存在）、`153`（OTP 已啟用）

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

---

## POST /auth/signin/otp
針對已啟用 OTP 的帳號，使用 Email、密碼與單一 6 碼驗證碼登入。伺服器會同時檢查輸入的數值是否為當期 OTP 或尚未使用的備援碼。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "email": "user@example.com",
  "password": "Password",
  "otp_code": "123456"
}
```

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `email` | string | ✅ | 合法 Email 格式，不能為空 |
| `password` | string | ✅ | 不能為空 |
| `otp_code` | string | ⭕ | 6 位數字，可輸入當期 OTP 或備援碼 |

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
  "error_code": "152"
}
```

**常見錯誤碼**：`124`、`128`、`129`、`130`、`150`、`151`、`152`

---

## POST /auth/delete/otp
停用 OTP 並刪除伺服器上的 OTP 秘鑰與備援碼，需提供合法的 `access_token`。

### Headers
- `Authorization: Bearer <access_token>`

### Request Body
無

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 失敗回應範例
```json
{
  "error_code": "154"
}
```

**常見錯誤碼**：`126`（token 失效）、`142`（使用者不存在）、`154`（尚未啟用 OTP）
