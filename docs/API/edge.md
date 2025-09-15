# 🖥️ Edge 裝置註冊 API

---

## 📌 Endpoint
`POST https://api.redsafe-tw.com/edge/reg`

## 📨 Headers
- `Content-Type: application/json`

## 📝 Request Body
```json
{
  "edge_id": "RED-AAAAAAAA",
  "version": "1.0.0",
  "edge_name": "EDGE_NAME"
}
```

### 參數說明
| 欄位        | 型別   | 必填 | 說明                        |
|-------------|--------|------|-----------------------------|
| `edge_id`   | string | ✅   | Edge 裝置唯一 ID            |
| `version`   | string | ✅   | Edge 裝置版本 (格式 X.Y.Z)  |
| `edge_name` | string | ❌   | Edge 裝置名稱 (可選填)      |

---

## 🔎 Response

✅ 成功回應
```json
{
  "edge_id": "RED-AAAAAAAA"
}
```

❌ 失敗回應
```json
{
  "error_code": "error_code"
}
```

---

## ⚠️ 常見錯誤碼
- `120` - Invalid Edge Id format  
- `121` - Edge version must follow format X.Y.Z (e.g., 1.0.0)  
- `122` - edge_name 為空白  
- `125` - edge_id 為空  
- `123` - edge_id 不存在  
