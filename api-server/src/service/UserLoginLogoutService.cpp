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

#include <nlohmann/json.hpp>
#include <sodium.h>
#include <vector>

#include "UserLoginLogoutService.hpp"
#include "../model/sql/FinderModels.hpp"
#include "../model/validator/ParameterValidation.hpp"

namespace redsafe::apiserver::service
{
    UserLoginLogoutService::UserLoginLogoutService(std::string email, std::string password)
        : email_(std::move(email)), password_(std::move(password))
    {
    }

    util::Result UserLoginLogoutService::login() const
    {
        using namespace model::validator;
        using namespace model::sql::fin;
        using json = nlohmann::json;

        if (!is_vaild_email(email_))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Invalid_email_format,
                json{}
            };

        const auto password_hash = UserPasswordHashFinder::start(email_);
        if (password_hash.empty())
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Email_or_Password_Error,
                json{}
            };

        if (!VerifyPassword(password_hash, password_))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Email_or_Password_Error,
                json{}
            };

        const auto user_id = UserIDFinder::start(email_);
        if (user_id.empty())
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        const auto user_name = UserNameFinder::start(email_);
        if (user_name.empty())
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        const auto serial_number = UserEdgeListFinder::start(user_id);

        return util::Result{
            util::status_code::Success,
            util::error_code::Success,
            json{
                {"user_id", user_id},
                {"user_name", user_name},
                {"serial_number", serial_number}
            }
        };
    }

    util::Result UserLoginLogoutService::logout()
    {
        return util::Result{
            util::status_code::BadRequest,
            util::error_code::Internal_server_error,
            nlohmann::json{}
        };
    }

    /// @brief  驗證明文密碼是否與儲存雜湊相符
    /// @param  pwdhash   從資料庫撈出的 Argon2id 雜湊字串
    /// @param  password  使用者輸入的明文密碼
    /// @return true = 密碼正確；false = 不符
    bool UserLoginLogoutService::VerifyPassword(const std::string &pwdhash, const std::string &password)
    {
        if (sodium_init() < 0)
            return false;
        // throw std::runtime_error("Failed to initialize sodium");

        // crypto_pwhash_str_verify 內部做常數時間比較 防止 timing attack
        return crypto_pwhash_str_verify(
                   pwdhash.c_str(),
                   password.c_str(),
                   password.size()
               ) == 0;
    }
}