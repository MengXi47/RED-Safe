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
    class IOSDeviceBindService
    {
    public:
        explicit IOSDeviceBindService(
            std::string serial_number,
            std::string ios_device_id);

        [[nodiscard]] json bind()   const;
        [[nodiscard]] json unbind() const;
    private:
        inline static const std::regex kSerialRe    {R"(^RED-[0-9A-F]{8}$)"};
        std::string serial_number_;
        std::string ios_device_id_;
    };
}