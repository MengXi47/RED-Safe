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

#include "UserLoginLogoutService.hpp"

#include <sodium.h>
#include <vector>

namespace redsafe::apiserver::service
{
    UserLoginLogoutService::UserLoginLogoutService(std::string email, std::string password)
        : email_(std::move(email)), password_(std::move(password))
    {
        if (!std::regex_match(email_, kEmailRe))
            throw std::invalid_argument("Invalid email format");
    }

    json UserLoginLogoutService::login() const
    {
        try
        {
            const std::string pwdhash =
                std::make_unique<model::sql::UserPasswordHashFinder>(email_)->FetchPasswordHash();
            if (!VerifyPassword(pwdhash, password_))
                throw std::runtime_error("Email or Password Error");

            const std::string user_id =
                std::make_unique<model::sql::UserIDFinder>(email_)->FetchUserId();

            const std::string user_name =
                std::make_unique<model::sql::UserNameFinder>(email_)->FetchUserName();

            const std::vector<std::string> serial_number =
                std::make_unique<model::sql::UserEdgeListFinder>(user_id)->FetchEdges();

            return json{
                {"status", "success"},
                {"user_id", user_id},
                {"user_name", user_name},
                {"serial_number", serial_number}
            };
        }
        catch ([[maybe_unused]] std::exception &e)
        {
            throw;
        }
    }

    json UserLoginLogoutService::logout()
    {
        return json{};
    }

    /// @brief  驗證明文密碼是否與儲存雜湊相符
    /// @param  pwdhash   從資料庫撈出的 Argon2id 雜湊字串
    /// @param  password  使用者輸入的明文密碼
    /// @return true = 密碼正確；false = 不符
    bool UserLoginLogoutService::VerifyPassword(std::string pwdhash, std::string password)
    {
        if (sodium_init() < 0)
            throw std::runtime_error("Failed to initialize sodium");

        // crypto_pwhash_str_verify 內部做常數時間比較 防止 timing attack
        return crypto_pwhash_str_verify(
            pwdhash.c_str(),
            password.c_str(),
            password.size()) == 0;
    }
}
