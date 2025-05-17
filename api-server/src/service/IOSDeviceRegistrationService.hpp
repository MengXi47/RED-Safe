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


#pragma once

#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

namespace redsafe::apiserver::service
{
    class IOSDeviceRegistrationService
    {
    public:
        explicit IOSDeviceRegistrationService(
            std::string ios_device_id,
            std::string user_id,
            std::string apns_token,
            std::string device_name);

        [[nodiscard]] json Register() const;
    private:
        inline static const std::regex kApnsRe{R"(^[0-9a-f]{64}$)"};
        std::string ios_device_id_;
        std::string user_id_;
        std::string apns_token_;
        std::string device_name_;
    };
}