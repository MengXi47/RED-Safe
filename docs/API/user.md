# 使用者相關 API

此檔案記錄與 `/user` 路徑相關的所有 API 介面，包括註冊、登入與取得所有使用者資訊等操作。

---

## 目錄

1. [使用者註冊](#使用者註冊)
2. [使用者登入](#使用者登入)
3. [取得所有使用者資訊](#取得所有使用者資訊)
4. [常見錯誤碼](#常見錯誤碼)

---

## 使用者註冊

<a name="使用者註冊"></a>

- **HTTP 方法**：`POST`
- **Endpoint**：`/user/signup`
- **CORS**：
    - 需允許 `Access-Control-Allow-Headers: Content-Type`
    - 需設定 `headers: { "Content-Type": "application/json" }`
    - 不需攜帶 `Authorization` 或 Cookie

### 功能說明

此接口用於新使用者註冊。前端送出 email、password，後端進行欄位驗證、email 是否重複檢查，若成功就將使用者資料寫入資料庫並回傳成功回應；若失敗則回傳對應錯誤碼。

### 必填參數

| 參數        | 類型   | 必填 | 說明                             |
|-------------|--------|------|--------------------------------|
| `email`     | String | 是   | 使用者電子郵件                        |
| `user_name` | String | 是   | 使用者暱稱/名稱                       |
| `password`  | String | 是   | 使用者密碼，至少 8 碼，英文大小寫各 1 碼，數字 1 碼 |

### Request 範例

```http
POST /user/signup HTTP/1.1
Host: api.redsafe-tw.com
Connection: keep-alive
Content-Type: application/json

{
  "email": "test@example.com",
  "user_name": "name",
  "password": "Password123"
}
```

### Response 範例 (成功：HTTP 200)

```http
HTTP/1.1 200 OK
Connection: keep-alive
Content-Type: application/json

{
  "email": "test@example.com",
  "user_name": "name",
  "error_code": 0
}
```

### Response 範例 (失敗：HTTP 400)


```http
HTTP/1.1 400 Bad Request
Connection: keep-alive
Content-Type: application/json

{
  "email": "test@example.com",
  "user_name": "name",
  "error_code": 200
}
```

---

## 使用者登入

<a name="使用者登入"></a>

- **HTTP 方法**：`POST`
- **Endpoint**：`/user/signin`
- **CORS**：  
    - 需允許 `Access-Control-Allow-Headers: Content-Type, Authorization`  
    - 前端呼叫時需設定 `credentials: "include"` 以便接收後端發送的 HttpOnly Cookie（Refresh Token）  

### 功能說明

此接口用於既有使用者登入。前端傳入 email 與 password，後端驗證成功後：
1. 於回應 JSON 中回傳 Access Token（`access_token`）。  
2. 於 `Set-Cookie` 標頭發送 Refresh Token（`refresh_token=<64hex>`，`HttpOnly; Secure; SameSite=Strict; Path=/auth; Max-Age=2592000`）。

前端需將 `access_token` 儲存並在日後請求帶入 `Authorization: Bearer <access_token>`；瀏覽器則自動管理 `refresh_token` Cookie，當執行 `/auth/refresh` 續期時自動夾帶。

### 必填參數

| 參數       | 類型   | 必填 | 說明           |
|------------|--------|------|----------------|
| `email`    | String | 是   | 使用者電子郵件 |
| `password` | String | 是   | 使用者密碼     |

### Request 範例

```http
POST /user/signin HTTP/1.1
Host: api.redsafe-tw.com
Connection: keep-alive
Content-Type: application/json

{
  "email": "test@example.com",
  "password": "Password123"
}
```

### Response 範例 (成功：HTTP 200)

```http
HTTP/1.1 200 OK
Connection: keep-alive
Content-Type: application/json
Set-Cookie: refresh_token=8e80bcc649f7477ec5fae9b70e6bda2f07ca6c45277540d91546b06f94e05817; Path=/auth; Max-Age=2592000; HttpOnly; Secure; SameSite=Strict

{
  "access_token": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3NDkwMzUwMjIsImlhdCI6MTc0OTAzNDQyMiwiaXNzIjoiUkVELVNhZmUiLCJzdWIiOiI4RjI0QzVBMDk4MTI4OTgwREE3RkZEQjU4MUQwMUYzODRDMzc0NTMyRkRFQkJGMUM2NDY2QUM1NjgyNjgyNTFCQzA1NTBGOTcwOEY3NDQxMkZGMDg2NDlDNzZGRTI3QUI1RjY0Q0E5NzJCNjIwMTAzQUI2QzY5Q0FGMzFEQzI4OCJ9.dZDvFQWVViOrdXDHJkekb5BTh3J1W-2I24CwtzGz26Y",
  "error_code": 0,
  "user_name": "name"
}
```
## 取得所有使用者資訊

<a name="取得所有使用者資訊"></a>

- **HTTP 方法**：`GET`
- **Endpoint**：`/user/all`
- **授權**：
    - 必須在 `Authorization` 標頭帶入有效 Access Token：`Bearer <access_token>`
    - 若缺少或 Access Token 過期，伺服器回傳 `401 Unauthorized`

### 功能說明

此接口回傳 **所有** 使用者的基本資訊。僅限具有管理權限的 Access Token 存取。  
每位使用者欄位包含：

- `user_name`：使用者名稱  
- `email`：用戶信箱  
- `serial_number`：裝置序號陣列（`array<string>`，一位使用者可能綁定多台裝置）

### Request 範例

```http
GET /user/all HTTP/1.1
Host: api.redsafe-tw.com
Authorization: Bearer eyJhbGciOiJIUzI1NiJ9...
```

### Response 範例 (成功：HTTP 200)

```http
HTTP/1.1 200 OK
Connection: keep-alive
Content-Type: application/json

{
  "error_code": 0,
  "user_name": "BoEn",
  "email": "test@example.com",
  "serial_number": ["RED-A1111111", "RED-B1111111"]
}
```

### Response 範例 (失敗：HTTP 401)

```http
HTTP/1.1 401 Unauthorized
Connection: keep-alive
Content-Type: application/json

{
  "error_code": 301
}
```

## 常見錯誤碼

| 錯誤碼 | 說明                               |
|-------|------------------------------------|
| 0     | 成功                               |
| 99    | 未知端點                           |
| 100   | JSON 格式錯誤                      |
| 101   | 序號格式錯誤                       |
| 102   | APNs Token 格式錯誤                |
| 103   | Email 格式錯誤                     |
| 104   | 使用者名稱格式錯誤                 |
| 105   | 密碼格式錯誤                       |
| 201   | Email 或密碼錯誤                   |
| 301   | 邊緣裝置已註冊                     |
| 302   | Email 已註冊                       |
| 303   | 綁定已存在                         |
| 401   | 缺少序號或版本                     |
| 402   | 缺少 Email、使用者名稱或密碼       |
| 403   | 缺少 Email 或密碼                  |
| 404   | 缺少 user_id 或 apns_token         |
| 405   | 缺少 user_id 或序號                |
| 406   | 缺少 Refresh Token                 |
| 407   | 缺少 Access Token                  |
| 500   | 伺服器內部錯誤                     |
| 501   | Refresh Token 已過期               |
| 502   | Refresh Token 無效                 |
| 503   | Access Token 已過期                |
| 504   | Access Token 無效                  |
