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


#include "UserRegistrationService.hpp"
#include "../model/model.hpp"

#include <iostream>
#include <regex>
#include <sodium.h>
#include <vector>

namespace redsafe::apiserver::service
{
    UserRegistrationService::UserRegistrationService(std::string email,
                                                     std::string user_name,
                                                     std::string password)
        : email_    (std::move(email)),
          user_name_(std::move(user_name)),
          password_ (std::move(password))
    {
        if (!std::regex_match(email_, kEmailRe))
            throw std::invalid_argument("Invalid email format");
        if (!std::regex_match(user_name_, kUserRe))
            throw std::invalid_argument("Invalid user_name format");
        if (!std::regex_match(password_, kPwdRe))
            // 8位密碼 至少一個大寫字母、一個小寫字母、一個數字與
            throw std::invalid_argument("Invalid password format");
    }

    json UserRegistrationService::Register() const
    {
        std::string passwordhash;
        try
        {
            passwordhash = PasswordHASH();
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << e.what() << std::endl;
        }

        if (!std::make_shared<model::sql::UserRegistrar>
            (email_, user_name_, passwordhash)->RegisterUser())
            throw std::invalid_argument("Registration failed");

        std::string user_id = std::make_shared<model::sql::UserIDFinder>
            (email_)->FetchUserId();
        if (user_id.empty())
            throw std::invalid_argument("Registration failed");

        return json{
            {"status", "success"},
            {"email", email_},
            {"user_name", user_name_},
            {"user_id", user_id}
        };
    }

    std::string UserRegistrationService::PasswordHASH() const
    {
        if (sodium_init() < 0)
            throw std::runtime_error("Failed to initialize password hashing library");
        std::vector<char> hashBuf(crypto_pwhash_STRBYTES);
        if (crypto_pwhash_str(
                hashBuf.data(),
                password_.c_str(), password_.size(),
                crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
            throw std::runtime_error("Password hashing failed");
        return {hashBuf.data()};
    }
}
