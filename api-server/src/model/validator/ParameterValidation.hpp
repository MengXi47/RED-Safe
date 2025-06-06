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

#ifndef REDSAFE_PARAMETER_VALIDATION_HPP
#define REDSAFE_PARAMETER_VALIDATION_HPP

#include <regex>
#include <string>

namespace redsafe::apiserver::model::validator {
// true : 合法
inline bool is_vaild_serial_number(const std::string_view serial_number) {
  return std::regex_match(
      serial_number.begin(),
      serial_number.end(),
      std::regex{R"(^RED-[0-9A-F]{8}$)"});
}

// true : 合法
inline bool is_vaild_apns(const std::string_view apns) {
  return std::regex_match(
      apns.begin(), apns.end(), std::regex{R"(^[0-9a-f]{64}$)"});
}

// true : 合法
inline bool is_vaild_email(const std::string_view email) {
  return std::regex_match(
      email.begin(),
      email.end(),
      std::regex{R"(^[A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,}$)"});
}

// trun : 合法
inline bool is_vaild_password(const std::string_view password) {
  return std::regex_match(
      password.begin(),
      password.end(),
      std::regex{R"(^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[A-Za-z\d]{8,}$)"});
}

// true : 合法
inline bool is_vaild_user_name(const std::string_view user_name) {
  return std::regex_match(
      user_name.begin(),
      user_name.end(),
      std::regex{R"(^[A-Za-z0-9._\-]{1,16}$)"});
}
} // namespace redsafe::apiserver::model::validator

#endif