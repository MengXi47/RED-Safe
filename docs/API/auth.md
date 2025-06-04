

# Token 續期 API

本檔案說明 `/auth` 路徑下的 API，目標在於利用 **Refresh Token** 取得新的 **Access Token**。目前僅有一個端點：`/auth/refresh`。

---

## 目錄
1. [刷新 Access Token `/auth/refresh`](#刷新-access-token-authrefresh)

---

## 刷新 Access Token `/auth/refresh`
<a name="刷新-access-token-authrefresh"></a>

- **HTTP 方法**：`POST`
- **Endpoint**：`/auth/refresh`
- **授權方式**：  
  - 必須在 Cookie 內攜帶 `refresh_token=<64hex>`  
    `Set-Cookie` 由 `/user/signin` 或 `/edge/signup` 等接口取得  
  - 前端呼叫時需設定 `credentials: "include"`，讓瀏覽器自動夾帶 HttpOnly Cookie

### 功能說明
當 **Access Token** 即將過期或已過期時，前端可呼叫 `/auth/refresh`。  
伺服器將驗證 `refresh_token` 是否有效、未過期且未被撤銷：  
1. 驗證成功 → 回傳新的 `access_token`，並將 `error_code` 設為 `0`。  
2. 若 `refresh_token` 過期或無效 → 回傳錯誤碼（`501/502`)，前端需引導使用者重新登入。  

### 必填參數

| 參數 | 類型 | 必填 | 說明                            |
|------|------|------|---------------------------------|
| *無* | —    | —    | 僅需在 Cookie 中帶 `refresh_token`|

### Request 範例  

```http
POST /auth/refresh HTTP/1.1
Host: api.redsafe-tw.com
Connection: keep-alive
Cookie: refresh_token=8e80bcc649f7477ec5fae9b70e6bda2f07ca6c45277540d91546b06f94e05817
```

> **注意：** 請求 body 不需攜帶任何 JSON；僅透過 Cookie 夾帶 `refresh_token`。

### Response 範例（成功：HTTP 200）

```http
HTTP/1.1 200 OK
Connection: keep-alive
Content-Type: application/json

{
  "access_token": "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3NDkwMzU5OTEsImlhdCI6MTc0OTAzNTM5MSwiaXNzIjoiUkVELVNhZmUiLCJzdWIiOiI4RjI0QzVBMDk4MTI4OTgwREE3RkZEQjU4MUQwMUYzODRDMzc0NTMyRkRFQkJGMUM2NDY2QUM1NjgyNjgyNTFCQzA1NTBGOTcwOEY3NDQxMkZGMDg2NDlDNzZGRTI3QUI1RjY0Q0E5NzJCNjIwMTAzQUI2QzY5Q0FGMzFEQzI4OCJ9.dZDvFQWVViOrdXDHJkekb5BTh3J1W-2I24CwtzGz26Y",
  "error_code": 0
}
```

### Response 範例（失敗：HTTP 400）

```http
HTTP/1.1 400 Bad Request
Content-Type: application/json

{
  "error_code": 406
}
```

---

> 前端應在 Access Token 驗證失敗 (`error_code=503/504`) 時，自動呼叫 `/auth/refresh`。<br>
> 如果 `/auth/refresh` 仍回傳錯誤 (`error_code!=0`)，代表使用者必須重新登入。