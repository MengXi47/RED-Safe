# 🔐 Auth API 文件

---

## 📝 使用者註冊 API

### Endpoint
`POST https://api.redsafe-tw.com/auth/signup`

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "email": "admin@gmail.com",
  "user_name": "admin",
  "password": "password"
}
```

#### 📌 參數說明
| 欄位        | 型別   | 必填 | 說明         |
|-------------|--------|------|--------------|
| `email`     | string | ✅   | 使用者 Email |
| `user_name` | string | ✅   | 使用者名稱   |
| `password`  | string | ✅   | 使用者密碼   |

### Response

✅ 成功回應
```json
{
  "user_id": "85f22dda-efc2-459d-b518-640400a69e8d",
  "user_name": "admin"
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

#### ⚠️ 常見錯誤碼
- `124` - email 格式錯誤  
- `129` - Email 為空  
- `130` - Password 為空  
- `131` - user_name 為空  
- `133` - Email 已存在  

---

## 🔑 使用者登入 API

### Endpoint
`POST https://api.redsafe-tw.com/auth/signin`

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "email": "admin@gmail.com",
  "password": "admin"
}
```

#### 📌 參數說明
| 欄位      | 型別   | 必填 | 說明         |
|-----------|--------|------|--------------|
| `email`   | string | ✅   | 使用者 Email |
| `password`| string | ✅   | 使用者密碼   |

### Response

✅ 成功回應
```json
{
  "user_name": "admin",
  "access_token": "eyJhbGciOiJIUzI1NiJ9...",
  "refresh_token": "XEEamaoT3xgoY5hkuo5x..."
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

#### ⚠️ 常見錯誤碼
- `124` - email 格式錯誤  
- `128` - 帳號密碼錯誤  
- `129` - Email 為空  
- `130` - Password 為空  

---

## ♻️ 刷新 Token API

### Endpoint
`POST https://api.redsafe-tw.com/auth/refresh`

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "refresh_token": "8r9kmmRMka9MxKlPdCW7Nxq1udPaWX1Yd9zx5a8wWMs"
}
```

#### 📌 參數說明
| 欄位           | 型別   | 必填 | 說明                |
|----------------|--------|------|---------------------|
| `refresh_token`| string | ✅   | 使用者 Refresh Token |

### Response

✅ 成功回應
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiJ9..."
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

#### ⚠️ 常見錯誤碼
- `132` - refresh_token 失效  
- `137` - refresh_token 為空  
