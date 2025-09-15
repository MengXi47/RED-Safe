# 👤 User API 文件

---

## 🔗 綁定使用者與 Edge 裝置 API

### Endpoint
`POST https://api.redsafe-tw.com/user/bind/{edge_id}`

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
無

### Response

✅ 成功回應
```json
{
  "error_code": "0"
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

#### ⚠️ 常見錯誤碼
- `123` - edge_id 不存在  
- `125` - edge_id 為空  
- `126` - access_token 失效  
- `127` - 缺少 access_token  
- `134` - user_id & edge_id 已綁定  

---

## 🔓 解除綁定使用者與 Edge 裝置 API

### Endpoint
`POST https://api.redsafe-tw.com/user/unbind/{edge_id}`

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
無

### Response

✅ 成功回應
```json
{
  "error_code": "0"
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

#### ⚠️ 常見錯誤碼
- `123` - edge_id 不存在  
- `125` - edge_id 為空  
- `126` - access_token 失效  
- `127` - 缺少 access_token  
- `135` - user_id & edge_id 未綁定  

---

## 📋 獲取使用者所有已綁定 Edge 裝置 API

### Endpoint
`GET https://api.redsafe-tw.com/user/list/edge_id`

### Headers
- `Authorization: Bearer {access_token}`

### Request Body
無

### Response

✅ 成功回應
```json
{
  "edge_id": [
    "RED-AAAAAAAA",
    "RED-BBBBBBBB"
  ]
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

#### ⚠️ 常見錯誤碼
- `126` - access_token 失效  
- `127` - 缺少 access_token  
- `136` - edge_id 數量為 0  

---

## ✏️ 修改 Edge 裝置名稱 API

### Endpoint
`POST https://api.redsafe-tw.com/user/update/edge_name`

### Headers
- `Content-Type: application/json`
- `Authorization: Bearer {access_token}`

### Request Body
```json
{
  "edge_name": "EDGE_NAME"
}
```

#### 📌 參數說明
| 欄位          | 型別     | 必填 | 說明     |
|-------------|--------|----|--------|
| `edge_name` | string | ✅  | 新的裝置名稱 |

### Response

✅ 成功回應
```json
{
  "error_code": "0"
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

#### ⚠️ 常見錯誤碼
- `122` - edge_name 為空白  
- `123` - edge_id 不存在  
- `125` - edge_id 為空  
- `126` - access_token 失效  
- `127` - 缺少 access_token  
