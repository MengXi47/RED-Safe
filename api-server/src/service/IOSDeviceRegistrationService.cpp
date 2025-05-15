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

#include "IOSDeviceRegistrationService.hpp"
#include "../model/model.hpp"


namespace redsafe::apiserver::service
{
    IOSDeviceRegistrationService::IOSDeviceRegistrationService(std::string user_id,
                                                               std::string apns_token,
                                                               std::string device_name,
                                                               std::string timestamp)
        : user_id_      (std::move(user_id)),
          apns_token_   (std::move(apns_token)),
          device_name_  (std::move(device_name)),
          timestamp_    (std::move(timestamp))
    {
        if (!std::regex_match(apns_token_, kApnsRe))
            throw std::invalid_argument("Invalid APNs token format");
    }

    json IOSDeviceRegistrationService::Register() const
    {
        if (!std::make_shared<model::sql::IOSDeviceRegistrar>
            (user_id_, apns_token_, device_name_, timestamp_)->RegisterIOSDevice())
            throw std::invalid_argument("Registration failed");

        std::string ios_device_id = std::make_shared<model::sql::IOSDeviceFinder>
            (apns_token_)->FetchIOSDeviceId();
        if (ios_device_id.empty())
            throw std::runtime_error("Failed to retrieve ios_device_id");

        return json{
            {"status", "success"},
            {"apns_token", apns_token_},
            {"ios_device_id", ios_device_id},
            {"registration_time", timestamp_}
        };
    }
}
