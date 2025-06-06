/******************************************************************************
Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
*******************************************************************************/

#ifndef REDSAFE_IOSAPP_SERVICE_CPP
#define REDSAFE_IOSAPP_SERVICE_CPP

#include "IOSAPPService.hpp"

#include <nlohmann/json.hpp>

#include "../model/sql/read.hpp"
#include "../model/sql/write.hpp"
#include "../model/validator/ParameterValidation.hpp"
#include "../util/response.hpp"

namespace redsafe::apiserver::service::IOSAPP {
util::Result IOSAPPService::start(const std::string &ios_device_id, const std::string &user_id,
                                  const std::string &apns_token, const std::string &device_name) {
  using namespace model::validator;
  using namespace model::sql::reg;
  using namespace model::sql::fin;
  using json = nlohmann::json;

  if (!is_vaild_apns(apns_token))
    return util::Result{util::status_code::BadRequest, util::error_code::Invalid_apnstoken_format,
                        json{}};

  if (IOSDeviceRegistrar::start(ios_device_id, user_id, apns_token, device_name) == 1)
    return util::Result{util::status_code::InternalServerError,
                        util::error_code::Internal_server_error, json{}};

  if (!ios_device_id.empty())
    return util::Result{
        util::status_code::Success, util::error_code::Success,
        json{{"user_id", user_id}, {"apns_token", apns_token}, {"ios_device_id", ios_device_id}}};

  if (auto ios_device_id_ = IOSDeviceIDFinder::start(apns_token); ios_device_id_.empty())
    return util::Result{util::status_code::InternalServerError,
                        util::error_code::Internal_server_error, json{}};
  else
    return util::Result{
        util::status_code::Success, util::error_code::Success,
        json{{"user_id", user_id}, {"apns_token", apns_token}, {"ios_device_id", ios_device_id_}}};
}
}  // namespace redsafe::apiserver::service::IOSAPP

#endif