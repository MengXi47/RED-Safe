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

namespace redsafe::server::model::validator {
// true : 合法
inline bool is_vaild_serial_number(const std::string_view serial_number) {
  return std::regex_match(
      serial_number.begin(),
      serial_number.end(),
      std::regex{R"(^RED-[0-9A-F]{8}$)"});
}
} // namespace redsafe::apiserver::model::validator

#endif