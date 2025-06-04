

# 邊緣裝置相關 API

此文件紀錄 `/edge` 路徑下的所有介面。

---

## 目錄
1. [邊緣裝置註冊](#邊緣裝置註冊-edgesignup)

---

## 邊緣裝置註冊
<a name="邊緣裝置註冊-edgesignup"></a>

- **HTTP 方法**：`POST`
- **Endpoint**：`/edge/signup`
- **授權**：此接口目前不需要 Access Token，但後端會驗證序號格式及版本號。

### 功能說明
用於首次將邊緣裝置序號寫入系統，並回傳後端標準化後的序號字串。若該裝置序號已存在，伺服器會回傳 `301`（邊緣裝置已註冊）。

### 必填參數

| 參數           | 類型   | 必填 | 說明                         |
|----------------|--------|------|------------------------------|
| `serial_number`| String | 是   | 裝置序號，格式為 `RED-XXXXNNNN` |
| `version`      | String | 是   | 固件/韌體版本，例如 `1.1.1` |

### Request 範例

```http
POST /edge/signup HTTP/1.1
Host: api.redsafe-tw.com
Connection: keep-alive
Content-Type: application/json

{
  "serial_number": "RED-A1111111",
  "version": "1.1.1"
}
```

### Response 範例（成功：HTTP 200）

```http
HTTP/1.1 200 OK
Connection: keep-alive
Content-Type: application/json

{
  "error_code": 0,
  "serial_number": "RED-AAAA1111"
}
```

### Response 範例（失敗：HTTP 400）

```http
HTTP/1.1 400 Bad Request
Content-Type: application/json

{
  "error_code": 101,
  "message": "序號格式錯誤"
}
```

### 常見錯誤碼

| 錯誤碼 | 說明                     |
|-------|--------------------------|
| 0     | 成功                     |
| 101   | 序號格式錯誤             |
| 301   | 邊緣裝置已註冊           |
| 401   | 缺少序號或版本           |
| 500   | 伺服器內部錯誤           |

---

> **備註**：裝置若需解绑請呼叫 `/edge/unbind`，格式請參考後續文件更新。 