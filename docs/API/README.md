# 專案文件總覽

歡迎閱讀本專案的文件（[`API/README.md`](README.md)）。此處收錄了 API 介面說明。

---

## 一、接口簡介

本專案名為 **RED-Safe API Server**，主要功能包括：
1. **使用者註冊/登入**：
    - [`/user/signup`](user.md#使用者註冊)： 使用者註冊
    - [`/user/signin`](user.md#使用者登入)： 使用者登入
2. **邊緣裝置管理**：
    - [`/edge/signup`](edge.md#邊緣裝置註冊)：邊緣裝置註冊
3. **iOS 裝置綁定**：
    - [`/ios/signup`](ios.md#iOS-裝置註冊)：裝置註冊
    - [`/ios/bind`](ios.md#iOS-裝置綁定)：裝置綁定
    - [`/ios/unbind`](ios.md#iOS-裝置解绑)：裝置解绑
4. **Token 續期**：
    - [`/auth/refresh`](auth.md#Token-續期)：Token-續期
5. **使用者資訊**：
    - [`/user/all`](user.md#取得所有使用者資訊)：取得所有使用者資訊
6. **錯誤碼**：
    - [`error_code`](error_code.md)：錯誤碼對應表
---

## 二、文件結構

- **[`API/user.md`](user.md)**： 使用者相關 API，例如註冊、登入、取得個人資料。
- **[`API/edge.md`](edge.md)**： 邊緣裝置註冊與查詢接口說明。
- **[`API/ios.md`](ios.md)**： iOS 裝置專屬的綁定 API。
- **[`API/auth.md`](auth.md)**： Access Token/Refresh Token 流程。  
