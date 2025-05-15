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
    IOSDeviceBindService::IOSDeviceBindService(std::string serial_number, std::string ios_device_id)
        : serial_number_(std::move(serial_number)), ios_device_id_(std::move(ios_device_id))
    {
        if (!std::regex_match(serial_number_, kSerialRe))
            throw std::invalid_argument("Invalid serial_number format");
    }

    json IOSDeviceBindService::bind() const
    {
        if (std::make_unique<model::sql::EdgeIOSBindingRegistrar>
            (serial_number_, ios_device_id_)->Bind())
            return json{
                    {"status", "success"},
                    {"serial_number", serial_number_},
                    {"ios_device_id", ios_device_id_}
            };
        throw std::invalid_argument("bind failed");
    }

    json IOSDeviceBindService::unbind() const
    {
        if (std::make_unique<model::sql::EdgeIOSBindingRegistrar>
            (serial_number_, ios_device_id_)->Unbind())
            return json{
                {"status", "success"},
                {"serial_number", serial_number_},
                {"ios_device_id", ios_device_id_}
            };
        throw std::invalid_argument("unbind failed");
    }
}
