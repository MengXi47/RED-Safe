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

#include <nlohmann/json.hpp>

#include "IOSDeviceBindService.hpp"
#include "../util/response.hpp"
#include "../model/sql/RegistrarModels.hpp"
#include "../model/validator/ParameterValidation.hpp"

namespace redsafe::apiserver::service
{
    util::Result IOSDeviceBindService::bind(const std::string &serial_number, const std::string &user_id)
    {
        using namespace model::validator;
        using namespace model::sql::reg;
        using json = nlohmann::json;

        if (!is_vaild_serial_number(serial_number))
            return {
                util::status_code::BadRequest,
                util::error_code::Invalid_serialnumber_format,
                json{}
            };

        if (const auto a = EdgeIOSBindingRegistrar::bind(serial_number, user_id); a == 1)
            return {
                util::status_code::Conflict,
                util::error_code::Binding_already_exists,
                json{}
            };
        else if (a == 2)
            return {
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        return {
            util::status_code::Success,
            util::error_code::Success,
            json{
                {"serial_number", serial_number},
                {"user_id", user_id}
            }
        };
    }

    util::Result IOSDeviceBindService::unbind(const std::string &serial_number, const std::string &user_id)
    {
        using namespace model::validator;
        using namespace model::sql::reg;
        using json = nlohmann::json;

        if (!is_vaild_serial_number(serial_number))
            return {
                util::status_code::BadRequest,
                util::error_code::Invalid_serialnumber_format,
                json{}
            };

        if (const auto a = EdgeIOSBindingRegistrar::unbind(serial_number, user_id); a == 1)
            return {
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        return util::Result{
            util::status_code::Success,
            util::error_code::Success,
            json{
                    {"serial_number", serial_number},
                    {"user_id", user_id}
            }
        };
    }
}