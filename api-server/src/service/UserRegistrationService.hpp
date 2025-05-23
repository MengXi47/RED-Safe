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

#include "../util/response.hpp"

namespace redsafe::apiserver::service
{
    class UserRegistrationService
    {
    public:
        explicit UserRegistrationService(
            std::string email,
            std::string user_name,
            std::string password);

        [[nodiscard]] util::Result start() const;
    private:
        [[nodiscard]] std::string PasswordHASH() const;
        std::string email_;
        std::string user_name_;
        std::string password_;
    };
}