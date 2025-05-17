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

#include "IOSDeviceBindService.hpp"

#include "../model/sql_model.hpp"

namespace redsafe::apiserver::service
{
    IOSDeviceBindService::IOSDeviceBindService(std::string serial_number, std::string user_id)
        : serial_number_(std::move(serial_number)), user_id_(std::move(user_id))
    {
        if (!std::regex_match(serial_number_, kSerialRe))
            throw std::invalid_argument("Invalid serial_number format");
    }

    json IOSDeviceBindService::bind() const
    {
        if (std::make_unique<model::sql::EdgeIOSBindingRegistrar>
            (serial_number_, user_id_)->bind())
            return json{
                            {"status", "success"},
                            {"serial_number", serial_number_},
                            {"user_id", user_id_}
            };
        throw std::invalid_argument("bind failed");
    }

    json IOSDeviceBindService::unbind() const
    {
        if (std::make_unique<model::sql::EdgeIOSBindingRegistrar>
            (serial_number_, user_id_)->unbind())
            return json{
                {"status", "success"},
                {"serial_number", serial_number_},
                {"user_id", user_id_}
            };
        throw std::invalid_argument("unbind failed");
    }
}
