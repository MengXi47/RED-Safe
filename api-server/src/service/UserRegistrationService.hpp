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

#pragma once

#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

namespace redsafe::apiserver::service
{
    class UserRegistrationService
    {
    public:
        explicit UserRegistrationService(
            std::string email,
            std::string user_name,
            std::string password);

        [[nodiscard]] json Register() const;
        [[nodiscard]] std::string PasswordHASH() const;
    private:
        inline static const std::regex kEmailRe {R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)"};
        inline static const std::regex kUserRe  {R"(^[A-Za-z0-9_-]{4,30}$)"};
        inline static const std::regex kPwdRe   {R"(^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[A-Za-z\d]{8,}$)"};
        std::string email_;
        std::string user_name_;
        std::string password_;
    };
}