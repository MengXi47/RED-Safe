# 🔐 Auth API 文件

Auth 服務負責使用者註冊、登入、二階段驗證 (OTP) 與信箱驗證。所有回應皆為 JSON。

---

## POST /auth/signup
建立全新的使用者帳號並同步建立 user profile。

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

| 欄位 | 型別 | 必填 | 說明 |
|------|------|------|------|
| `email` | string | ✅ | 會轉為小寫；需符合 Email 格式 |
| `user_name` | string | ✅ | 最長 16 字元 |
| `password` | string | ✅ | 伺服器端會雜湊儲存 |

### 成功回應 (200)
```json
{
  "user_id": "85f22dda-efc2-459d-b518-640400a69e8d",
  "user_name": "USER_NAME"
}
```

### 常見錯誤碼
- `124` Email 格式錯誤
- `129` Email 為空
- `130` Password 為空
- `131` user_name 為空
- `133` Email 已存在
- `139` user_name 過長

---

## POST /auth/signin
使用 Email 與密碼登入。若帳號已啟用二階段驗證，會以 `error_code` 回應通知改走 OTP 流程。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "email": "user@example.com",
  "password": "Password123"
}
```

### 成功回應 (200)
```json
{
  "user_name": "USER_NAME",
  "access_token": "eyJhbGciOiJIUzI1NiJ9...",
  "refresh_token": "ajh23kjasd..."
}
```

### OTP 啟用時的回應 (200)
```json
{
  "error_code": "150"
}
```

### 常見錯誤碼
- `124`、`128` 帳號或密碼錯誤
- `150` 需要二階段認證 OTP

---

## POST /auth/signin/otp
適用已啟用二階段驗證的帳號，需提供一次性 6 碼驗證碼或備援碼。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "email": "user@example.com",
  "password": "Password123",
  "otp_code": "123456"
}
```

| 欄位 | 型別 | 必填 | 說明 |
|------|------|------|------|
| `otp_code` | string | ✅ | 可輸入同步產生的 OTP 或尚未使用的備援碼 |

### 成功回應 (200)
```json
{
  "user_name": "USER_NAME",
  "access_token": "eyJhbGciOiJIUzI1NiJ9...",
  "refresh_token": "ajh23kjasd..."
}
```

### 常見錯誤碼
- `124`、`128` 帳號或密碼錯誤
- `151` OTP 未啟用
- `152` OTP 驗證失敗

---

## POST /auth/refresh
以 Refresh Token 換取新的 Access Token。舊的存活時間不會被刷新。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "refresh_token": "rFT4nq9J..."
}
```

### 成功回應 (200)
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiJ9..."
}
```

### 常見錯誤碼
- `132` refresh_token 失效
- `137` refresh_token 為空

---

## POST /auth/create/otp
產生 TOTP 秘鑰並啟用二階段驗證。需提供有效的 Access Token。

### Headers
- `Authorization: Bearer {access_token}`

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

### 常見錯誤碼
- `126` access_token 失效
- `127` 缺少 access_token
- `142` user 不存在
- `153` 已啟用 OTP

---

## POST /auth/delete/otp
停用二階段驗證。需提供有效的 Access Token。

### Headers
- `Authorization: Bearer {access_token}`

### Request Body
無

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `126` access_token 失效
- `127` 缺少 access_token
- `142` user 不存在
- `154` 已停用 OTP

---

## POST /auth/mail/verify/send
寄送 Email 驗證碼。呼叫者必須提供目標 `user_id`。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "user_id": "85f22dda-efc2-459d-b518-640400a69e8d"
}
```

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `142` user 不存在
- `157` user_id 為空
- `NOTIFY_GRPC_FAILED` 郵件服務呼叫失敗

---

## POST /auth/mail/verify
驗證 Email 驗證碼並標記帳號為已驗證。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "user_id": "85f22dda-efc2-459d-b518-640400a69e8d",
  "code": "123456"
}
```

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 常見錯誤碼
- `142` user 不存在
- `156` email 驗證碼錯誤
- `157` user_id 為空
