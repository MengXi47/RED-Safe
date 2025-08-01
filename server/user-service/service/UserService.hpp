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
******************************************************************************/

#ifndef REDSAFE_USER_SERVICE_HPP
#define REDSAFE_USER_SERVICE_HPP

#include "../util/response.hpp"

namespace redsafe::server::service::User {
class signup {
 public:
  static util::Result run(
      const std::string& email,
      const std::string& user_name,
      const std::string& password);
};

class signin {
 public:
  /**
   * @brief 使用 emal 和 password 登入
   * @param email
   * @param password
   */
  static util::Result run(
      const std::string& email, const std::string& password);
};

class Binding {
 public:
  static util::Result bind(
      const std::string& serial_number, const std::string& access_token);
  static util::Result unbind(
      const std::string& serial_number, const std::string& access_token);
};

/**
 * @brief 獲取使用者資訊
 */
class GetUserInformation {
 public:
  /**
   * @param access_token
   * @return util::Result
   */
  static util::Result run(const std::string& access_token);

  /// 禁止拷貝和移動
  GetUserInformation(const GetUserInformation&) = delete;
  GetUserInformation(GetUserInformation&&) = delete;
  GetUserInformation& operator=(const GetUserInformation&) = delete;
  GetUserInformation& operator=(GetUserInformation&&) = delete;
};
} // namespace redsafe::apiserver::service::User

#endif