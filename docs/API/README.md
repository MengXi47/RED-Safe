# 🚀 RED Safe API 導航

Base URL：`https://api.redsafe-tw.com`

---

## 🔐 Auth 認證服務
- [註冊使用者](auth.md#post-authsignup)
- [使用者登入](auth.md#post-authsignin)
- [刷新 Access Token](auth.md#post-authrefresh)
- [登入（二階段驗證）](auth.md#post-authsigninotp)
- [啟用二階段驗證](auth.md#post-authcreateotp)
- [停用二階段驗證](auth.md#post-authdeleteotp)
- [寄送信箱驗證碼](auth.md#post-authmailverifysend)
- [驗證信箱驗證碼](auth.md#post-authmailverify)

## 👤 User 使用者服務
- [取得使用者資訊](user.md#get-userinfo)
- [綁定 Edge 裝置](user.md#post-userbind)
- [解除綁定 Edge 裝置](user.md#post-userunbindedge_id)
- [查詢綁定的 Edge 裝置](user.md#get-userlistedge_id)
- [更新 Edge 裝置名稱](user.md#post-userupdateedge_name)
- [更新使用者名稱](user.md#post-userupdateuser_name)
- [更新使用者密碼](user.md#post-userupdatepassword)
- [更新 Edge 裝置密碼](user.md#post-userupdateedge_password)
- [發送 Edge 指令](user.md#post-useredgecommand)
- [訂閱 Edge 指令回傳 (SSE)](user.md#get-userssegetcommandtrace_id)
- [查詢使用者編號](user.md#get-useruserid)

## 🖥️ Edge 裝置服務
- [Edge 裝置註冊](edge.md#post-edgereg)
- [Edge 裝置密碼更新](edge.md#post-edgeupdateedge_password)
- [Edge 裝置上線回報](edge.md#post-edgeonline)
- [查詢綁定使用者列表](edge.md#get-edgeuserlist)
- [解除綁定指定使用者](edge.md#post-edgeuserunbind)

## 📱 iOS 裝置服務
- [註冊或更新 iOS 裝置](ios.md#post-iosreg)

## ⚠️ 通用錯誤碼
- [錯誤碼總覽](error_code.md#錯誤碼)
- [iOS 服務錯誤識別碼](error_code.md#ios-服務錯誤識別碼)
