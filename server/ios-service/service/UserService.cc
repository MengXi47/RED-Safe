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

#include "UserService.hpp"

#include <nlohmann/json.hpp>

#include "../model/sql/read.hpp"
#include "../model/sql/write.hpp"
#include "../model/validator/ParameterValidation.hpp"
#include "TokenService.hpp"

namespace redsafe::server::service::User {
using namespace model::validator;
using namespace model::sql::reg;
using namespace model::sql::fin;
using json = nlohmann::json;

util::Result Binding::bind(
    const std::string& serial_number, const std::string& access_token) {
  token::DecodeAccessToken decode(access_token);
  {
    const auto code = decode.start();

    if (code == 1) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::Access_Token_expired,
          json{}};
    }
    if (code == 2) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::Access_Token_invalid,
          json{}};
    }
    if (code == 3) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::JWT_invalid_signature,
          json{}};
    }
    if (code == 4) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::JWT_invalid_token_supplied,
          json{}};
    }
    if (code == 5) {
      return util::Result{
          util::status_code::InternalServerError,
          util::error_code::Internal_server_error,
          json{}};
    }
  }

  if (!is_vaild_serial_number(serial_number)) {
    return util::Result{
        util::status_code::BadRequest,
        util::error_code::Invalid_serialnumber_format,
        json{}};
  }

  if (const auto a =
          EdgeIOSBindingRegistrar::bind(serial_number, decode.getUserId());
      a == 1) {
    return util::Result{
        util::status_code::Conflict,
        util::error_code::Binding_already_exists,
        json{}};
  } else if (a == 2) {
    return util::Result{
        util::status_code::InternalServerError,
        util::error_code::Internal_server_error,
        json{}};
  }

  return util::Result{
      util::status_code::Success,
      util::error_code::Success,
      json{{"serial_number", serial_number}}};
}

util::Result Binding::unbind(
    const std::string& serial_number, const std::string& access_token) {
  token::DecodeAccessToken decode(access_token);
  {
    const auto code = decode.start();

    if (code == 1) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::Access_Token_expired,
          json{}};
    }
    if (code == 2) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::Access_Token_invalid,
          json{}};
    }
    if (code == 3) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::JWT_invalid_signature,
          json{}};
    }
    if (code == 4) {
      return util::Result{
          util::status_code::BadRequest,
          util::error_code::JWT_invalid_token_supplied,
          json{}};
    }
    if (code == 5) {
      return util::Result{
          util::status_code::InternalServerError,
          util::error_code::Internal_server_error,
          json{}};
    }
  }

  if (!is_vaild_serial_number(serial_number)) {
    return util::Result{
        util::status_code::BadRequest,
        util::error_code::Invalid_serialnumber_format,
        json{}};
  }

  if (const auto a =
          EdgeIOSBindingRegistrar::unbind(serial_number, decode.getUserId());
      a == 1) {
    return util::Result{
        util::status_code::InternalServerError,
        util::error_code::Internal_server_error,
        json{}};
  }

  return util::Result{
      util::status_code::Success,
      util::error_code::Success,
      json{{"serial_number", serial_number}}};
}
} // namespace redsafe::server::service::User
