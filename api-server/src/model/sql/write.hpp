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

#ifndef REDSAFE_REGISTRAR_MODEL_HPP
#define REDSAFE_REGISTRAR_MODEL_HPP

#include "SqlConnectionManager.hpp"

namespace redsafe::apiserver::model::sql::reg {
// 邊緣裝置註冊
class EdgeDeviceRegistrar : public ConnectionManager {
 public:
  // 0:成功 1:serial_number註冊過 2:SQL錯誤
  [[nodiscard]] static int start(std::string_view serial_number,
                                 std::string_view version);
};

// 使用者註冊
class UserRegistrar : public ConnectionManager {
 public:
  // 0:成功 1:email被註冊過 2:SQL錯誤
  [[nodiscard]] static int start(std::string_view email,
                                 std::string_view user_name,
                                 std::string_view password_hash);
};

// IOS裝置註冊
class IOSDeviceRegistrar : public ConnectionManager {
 public:
  // 0:成功 1:SQL錯誤
  [[nodiscard]] static int start(std::string_view ios_device_id,
                                 std::string_view user_id,
                                 std::string_view apns_token,
                                 std::string_view device_name);
};

// Edge ↔ iOS 裝置綁定
class EdgeIOSBindingRegistrar : public ConnectionManager {
 public:
  // 0:成功 1:已經綁定 2:SQL錯誤
  [[nodiscard]] static int bind(std::string_view edge_serial_number,
                                std::string_view user_id);

  // 0:成功 1:SQL錯誤
  [[nodiscard]] static int unbind(std::string_view edge_serial_number,
                                  std::string_view user_id);
};

// Refresh Token 註冊
class RefreshTokenRegistrar : public ConnectionManager {
 public:
  // 0:成功 1:refresh_token_hash 已存在 2:SQL錯誤
  [[nodiscard]] static int start(std::string_view refresh_token_hash,
                                 std::string_view user_id);
};

// Refresh Token 註銷
class RefreshTokenRevoke : public ConnectionManager {
 public:
  // 0:成功 1:SQL錯誤
  [[nodiscard]] static int start(std::string_view refresh_token_hash);
};
}  // namespace redsafe::apiserver::model::sql::reg

#endif