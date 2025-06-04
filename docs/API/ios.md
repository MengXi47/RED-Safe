

# iOS 裝置相關 API

本文檔記錄 `/ios` 路徑下的介面，主要用於 iOS 裝置註冊、綁定與解绑。本頁僅先補上 **`/ios/signup`** 端點，其他端點可依此格式持續擴充。

---

## 目錄
1. [iOS 裝置註冊](#iOS-裝置註冊)

---

## iOS 裝置註冊
<a name="iOS-裝置註冊"></a>

- **HTTP 方法**：`POST`
- **Endpoint**：`/ios/signup`
- **授權**：
  - 必須在 HTTP 標頭帶入 **Access Token**：  
    `Authorization: Bearer <access_token>`

### 功能說明
當 iOS 裝置首次啟動 App 時，若未曾註冊過，需呼叫此端點以「建立裝置資料」。  
- **第一次註冊**：`ios_device_id` 可傳空字串，伺服器會分配新的 `ios_device_id`。  
- **後續上線**：若已分配過 `ios_device_id`，則必須在後續請求中帶上該 ID，以避免重複註冊。  

### 必填參數

| 參數           | 類型   | 必填 | 說明                                           |
|----------------|--------|------|-----------------------------------------------|
| `device_name`  | String | 是   | 裝置名稱              |
| `apns_token`   | String | 是   | Apple Push Notification Token            |
| `ios_device_id`| String | 否   | iOS 裝置唯一識別碼。第一次註冊可空字串，之後需帶上 |

### Request 範例

```http
POST /ios/signup HTTP/1.1
Host: api.redsafe-tw.com
Authorization: Bearer eyJhbGciOiJIUzI1NiJ9...
Content-Type: application/json

{
  "device_name": "admin 的 iPhone",
  "apns_token": "abcdefg1234567...",
  "ios_device_id": ""
}
```

### Response 範例（成功：HTTP 200）

```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "error_code": 0,
  "ios_device_id": "e1d23a56-789b-4ef0-bc12-3456789abcde",
  "apns_token": "abcdefg1234567..."
}
```

### Response 範例（失敗：HTTP 400）

```http
HTTP/1.1 400 Bad Request
Content-Type: application/json

{
  "error_code": 100
}
```
