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

#ifndef REDSAFE_RESPONSE_UTIL_HPP
#define REDSAFE_RESPONSE_UTIL_HPP

#include "folly_json.hpp"

namespace redsafe::apiserver::util {
enum class status_code : int {
  Success = 200,
  Created = 201,
  BadRequest = 400,
  Unauthorized = 401,
  Forbidden = 403,
  NotFound = 404,
  Conflict = 409,
  UnprocessableEntity = 422,
  InternalServerError = 500,
  ServiceUnavailable = 503
};

enum class error_code : int {
  Success = 0,
  Unknown_endpoint = 99,
  Invalid_JSON = 100,
  Invalid_serialnumber_format = 101,
  Invalid_apnstoken_format = 102,
  Invalid_email_format = 103,
  Invalid_username_format = 104,
  Invalid_password_format = 105,
  Email_or_Password_Error = 201,
  Edge_device_already_registered = 301,
  Email_already_registered = 302,
  Binding_already_exists = 303,
  Missing_serial_number_or_version = 401,
  Missing_email_or_user_name_or_password = 402,
  Missing_email_or_password = 403,
  Missing_user_id_or_apns_token = 404,
  Missing_serial_number = 405,
  Missing_refresh_token = 406,
  Missing_access_token = 407,
  Internal_server_error = 500,
  Refresh_Token_Expired = 501,
  Refresh_Token_invalid = 502,
  Access_Token_expired = 503,
  Access_Token_invalid = 504,
  JWT_invalid_signature = 505,
  JWT_invalid_token_supplied = 506
};

struct Result {
  status_code sc;
  error_code ec;
  json body;
  std::string token{};
};
}  // namespace redsafe::apiserver::util

#endif