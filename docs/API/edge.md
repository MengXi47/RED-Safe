# 🖥️ Edge 服務 API 文件

Edge 服務提供邊緣裝置註冊與密碼維護功能。

---

## POST /edge/reg
註冊全新的 Edge 裝置並設定初始密碼。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "edge_password": "EdgeSecret",
  "version": "1.0.0"
}
```

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `edge_id` | string | ✅ | 格式 `RED-XXXXXXXX`，大寫十六進位 |
| `edge_password` | string | ✅ | 不能為空 |
| `version` | string | ✅ | `X.Y.Z` 版本格式，不能為空 |

### 成功回應 (200)
```json
{
  "edge_id": "RED-1A2B3C4D"
}
```

### 失敗回應範例
```json
{
  "error_code": "146"
}
```

**常見錯誤碼**：`120`、`121`、`125`、`144`、`145`、`146`

---

## POST /edge/update/edge_password
由 Edge 裝置直接更新自身密碼。

### Headers
- `Content-Type: application/json`

### Request Body
```json
{
  "edge_id": "RED-1A2B3C4D",
  "edge_password": "CurrentEdgeSecret",
  "new_edge_password": "NextEdgeSecret"
}
```

| 欄位 | 型別 | 必填 | 約束 |
|------|------|------|------|
| `edge_id` | string | ✅ | 格式 `RED-XXXXXXXX` |
| `edge_password` | string | ✅ | 不能為空 |
| `new_edge_password` | string | ✅ | 不能為空 |

### 成功回應 (200)
```json
{
  "error_code": "0"
}
```

### 失敗回應範例
```json
{
  "error_code": "147"
}
```

**常見錯誤碼**：`120`、`123`、`125`、`144`、`147`、`148`
