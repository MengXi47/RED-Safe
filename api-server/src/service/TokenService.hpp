/******************************************************************************
Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
******************************************************************************/

#ifndef REDSAFE_TOKEN_SERVICE_HPP
#define REDSAFE_TOKEN_SERVICE_HPP

#include "../lib/jwt-cpp/jwt.h"
#include "../util/crypto.hpp"
#include "../util/response.hpp"

namespace redsafe::apiserver::service::token {
/**
 * @brief 創建 Access Token（JWT）
 */
class CreateAccessToken {
 public:
  /**
   * @brief 創建 Access_Token 使用 JWT
   *
   * @param user_id 使用者 user_id
   */
  explicit CreateAccessToken(std::string_view user_id);

  /**
   * @brief 執行解碼與驗證
   * @return 0: 驗證通過，並將 AccessToken 存到 token
   * @return 1: 例外錯誤
   */
  [[nodiscard]] int start();

  /**
   * @brief 取得編碼後的 AccessToken
   * @return 編碼後的 AccessToken，若未通過驗證則為空字串
   */
  [[nodiscard]] std::string getAccessToken() const;

  /**
   * @brief 取得驗證失敗時的錯誤訊息
   * @return 失敗時的錯誤描述
   */
  [[nodiscard]] std::string getErrorMessage() const;

 private:
  std::string user_id;
  std::string token;
  std::string errorMessage;
};

/**
 * @brief 解碼並驗證 Access Token（JWT）
 */
class DecodeAccessToken {
 public:
  /**
   * @brief 建構子，接收要解碼的 JWT 字串
   * @param tokenStr 要解碼的 Access Token
   */
  explicit DecodeAccessToken(std::string tokenStr);

  /**
   * @brief 執行解碼與驗證
   * @return 0: 驗證通過，並將 user_id 存到 payloadUserId
   * @return 1: 表示驗證失敗 過時
   * @return 2: Token 中缺少 sub 欄位
   * @return 3: invalid signature 簽名錯誤
   * @return 4: 例外錯誤
   */
  [[nodiscard]] int start();

  /**
   * @brief 取得解碼後的 user_id (sub claim)
   * @return std::string 解碼後的 user_id，若未通過驗證則為空字串
   */
  [[nodiscard]] std::string getUserId() const;

  /**
   * @brief 取得驗證失敗時的錯誤訊息
   * @return std::string 解驗證失敗時的錯誤描述
   */
  [[nodiscard]] std::string getErrorMessage() const;

 private:
  std::string tokenValue;
  std::string payloadUserId;
  std::string errorMessage;
};

/**
 * @brief 創建 Refresh Token
 */
class CreateRefreshToken {
 public:
  /**
   * @brief 創建 Refresh_Token
   *
   * @param user_id 使用者 user_id
   */
  explicit CreateRefreshToken(std::string_view user_id);

  /**
   * @brief 執行解碼與驗證
   * @return 0: 創建成功，並將 RefreshToken 存到 token
   * @return 1: 錯誤
   */
  [[nodiscard]] int start();

  /**
   * @brief 取得編碼後的 RefreshToken
   * @return 編碼後的 RefreshToken
   */
  [[nodiscard]] std::string getRefreshToken() const;

  /**
   * @brief 如果 `start()` 遇到例外，這裡會有錯誤訊息
   * @return 失敗時的錯誤描述；產生成功則為空字串
   */
  [[nodiscard]] std::string getErrorMessage() const;

 private:
  /**
   * @brief 將 Refresh 寫入資料庫
   */
  void WriteToSQL() const;
  std::string user_id;
  std::string token;
  std::string errorMessage;
};

/**
 * @brief check Refresh Token 是否過期或被註銷
 */
class CheckAndRefreshRefreshToken {
 public:
  /**
   * @brief 執行解碼與驗證
   * @return util::Result
   */
  [[nodiscard]] static util::Result run(const std::string& refreshtoken);

  /// 禁止拷貝和移動
  CheckAndRefreshRefreshToken(const CheckAndRefreshRefreshToken&) = delete;
  CheckAndRefreshRefreshToken(CheckAndRefreshRefreshToken&&) = delete;
  CheckAndRefreshRefreshToken& operator=(const CheckAndRefreshRefreshToken&) = delete;
  CheckAndRefreshRefreshToken& operator=(CheckAndRefreshRefreshToken&&) = delete;
};

/**
 * @brief Refresh Token 註銷
 */
class RevokeRefreshToken {
 public:
  /**
   * @brief 執行註銷
   * @return util::Result
   */
  [[nodiscard]] static util::Result run(const std::string& refreshtoken);

  /// 禁止拷貝和移動
  RevokeRefreshToken(const RevokeRefreshToken&) = delete;
  RevokeRefreshToken(RevokeRefreshToken&&) = delete;
  RevokeRefreshToken& operator=(const RevokeRefreshToken&) = delete;
  RevokeRefreshToken& operator=(RevokeRefreshToken&&) = delete;
};
}  // namespace redsafe::apiserver::service::token

#endif