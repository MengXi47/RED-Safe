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

#include "EdgeDeviceRegistrationService.hpp"
#include "../model/model.hpp"

#include <utility>

namespace redsafe::apiserver::service
{
    EdgeDeviceRegistrationService::EdgeDeviceRegistrationService(std::string version,
                                                                 std::string serial_number)
        : version_(std::move(version)), serial_number_(std::move(serial_number))
    {
        if (!std::regex_match(serial_number_, kSerialRe))
            throw std::invalid_argument("Invalid serial_number format");
    }

    json EdgeDeviceRegistrationService::Register() const
    {
        try
        {
            if (std::make_shared<model::sql::EdgeDeviceRegistrar>
                (serial_number_, version_)->RegisterEdgeDevice())
                return json{{"status", "success"}, {"serial_number", serial_number_}};
            throw std::invalid_argument("Registration failed");
        }
        catch ([[maybe_unused]] const std::runtime_error &e)
        {
            throw;
        }
        catch ([[maybe_unused]] const std::invalid_argument &e)
        {
            throw;
        }
    }
}
