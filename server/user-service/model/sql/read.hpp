/******************************************************************************
Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights
Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly
prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source
code;
     3. Distribute, display, or otherwise use this source code or its
derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
*******************************************************************************/

#ifndef REDSAFE_FINDER_MODEL_HPP
#define REDSAFE_FINDER_MODEL_HPP

#include "SqlConnectionManager.hpp"

namespace redsafe::server::model::sql::fin {
// 取得 User ID
class UserIDFinder : public ConnectionManager {
 public:
  // 成功回傳 User ID 否則空字串
  static std::string start(std::string_view email);
};

// 取得 User Name
class UserNameFinder : public ConnectionManager {
 public:
  // 成功回傳 User Name 否則空字串
  static std::string start_by_email(std::string_view email);

  // 成功回傳 User Name 否則空字串
  static std::string start_by_user_id(std::string_view user_id);
};

// 取得 Email
class UserEmailFinder : public ConnectionManager {
 public:
  // 成功回傳 Email 否則空字串
  static std::string start(std::string_view user_id);
};

// 取得 iOS device ID
class IOSDeviceIDFinder : public ConnectionManager {
 public:
  // 成功回傳iOS device ID 否則空字串
  static std::string start(std::string_view apns_token);
};

// 依 email 取得密碼雜湊
class UserPasswordHashFinder : public ConnectionManager {
 public:
  // 成功回傳password hash 否則空字串
  static std::string start(std::string_view email);
};

// 依 user_id 取回該帳號所有 Edge 序號
class UserEdgeListFinder : public ConnectionManager {
 public:
  // 成功回傳Edge 序號 ALL 否則空字串
  static std::vector<std::string> start(std::string_view user_id);
};

// 檢查並刷新 Refresh Token（若仍有效則續期 +30 天，回傳 user_id）
class RefreshTokenRefresher : public ConnectionManager {
 public:
  /**
   * @brief 驗證 refresh_token_hash 是否有效
   *        若有效：更新 expires_at = NOW() + INTERVAL '30 days' 並回傳 user_id
   *        若過期：自動 revoked = TRUE，回傳空字串
   *
   * @param refresh_token_hash Token 雜湊
   * @return std::string 有效則為 user_id，否則為空字串
   */
  static std::string start(std::string_view refresh_token_hash);
};
} // namespace redsafe::apiserver::model::sql::fin

#endif