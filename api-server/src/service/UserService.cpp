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

#include "UserService.hpp"
#include "../model/sql/FinderModels.hpp"
#include "../model/sql/RegistrarModels.hpp"
#include "../model/validator/ParameterValidation.hpp"
#include "../util/IOstream.hpp"
#include "../util/logger.hpp"

namespace redsafe::apiserver::service::User
{
    util::Result signin::start(
        const std::string& email,
        const std::string& password)
    {
        using namespace model::validator;
        using namespace model::sql::fin;
        using json = nlohmann::json;

        if (!is_vaild_email(email))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Invalid_email_format,
                json{}
            };

        const auto password_hash = UserPasswordHashFinder::start(email);
        if (password_hash.empty())
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Email_or_Password_Error,
                json{}
            };

        if (!VerifyPassword(password_hash, password))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Email_or_Password_Error,
                json{}
            };

        const auto user_id = UserIDFinder::start(email);
        if (user_id.empty())
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        const auto user_name = UserNameFinder::start(email);
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

    /// @brief  驗證明文密碼是否與儲存雜湊相符
    /// @param  pwdhash   從資料庫撈出的 Argon2id 雜湊字串
    /// @param  password  使用者輸入的明文密碼
    /// @return true = 密碼正確；false = 不符
    bool signin::VerifyPassword(const std::string &pwdhash, const std::string_view password)
    {
        if (sodium_init() < 0)
            return false;
        // throw std::runtime_error("Failed to initialize sodium");

        // crypto_pwhash_str_verify 內部做常數時間比較 防止 timing attack
        return crypto_pwhash_str_verify(
                   pwdhash.c_str(),
                   password.data(),
                   password.size()
               ) == 0;
    }

    util::Result signup::start(
            const std::string& email,
            const std::string& user_name,
            const std::string& password)
    {
        using namespace model::validator;
        using namespace model::sql::reg;
        using namespace model::sql::fin;
        using json = nlohmann::json;

        if (!is_vaild_email(email))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Invalid_email_format,
                json{}
            };

        // if (!is_vaild_user_name(user_name_))
        //     return util::Result{
        //         util::status_code::BadRequest,
        //         util::error_code::Invalid_username_format,
        //         json{}
        //     };

        if (!is_vaild_password(password))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Invalid_password_format,
                json{}
            };

        std::string password_hash;
        try
        {
            password_hash = PasswordHASH(password);
        }
        catch (const std::exception &e)
        {
            util::cerr() << e.what() << std::endl;
            util::log(util::LogFile::server, util::Level::ERROR) << e.what();
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };
        }

        if (const auto e = UserRegistrar::start(email, user_name, password_hash); e == 1)
            return util::Result{
                util::status_code::Conflict,
                util::error_code::Email_already_registered,
                json{}
            };
        else if (e == 2)
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        if (const auto user_id = UserIDFinder::start(email); user_id.empty())
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };
        else
            return util::Result{
                util::status_code::Success,
                util::error_code::Success,
                json{
                        {"email", email},
                        {"user_name", user_name},
                        {"user_id", user_id}
                }
            };
    }

    std::string signup::PasswordHASH(std::string_view password)
    {
        if (sodium_init() < 0)
            throw std::runtime_error("Failed to initialize password hashing library");
        std::vector<char> hashBuf(crypto_pwhash_STRBYTES);
        if (crypto_pwhash_str(
                hashBuf.data(),
                password.data(), password.size(),
                crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE
            ) != 0)
            throw std::runtime_error("Password hashing failed");
        return {hashBuf.data()};
    }

    util::Result Bind::bind(const std::string &serial_number, const std::string &user_id)
    {
        using namespace model::validator;
        using namespace model::sql::reg;
        using json = nlohmann::json;

        if (!is_vaild_serial_number(serial_number))
            return {
                util::status_code::BadRequest,
                util::error_code::Invalid_serialnumber_format,
                json{}
            };

        if (const auto a = EdgeIOSBindingRegistrar::bind(serial_number, user_id); a == 1)
            return {
                util::status_code::Conflict,
                util::error_code::Binding_already_exists,
                json{}
            };
        else if (a == 2)
            return {
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        return {
            util::status_code::Success,
            util::error_code::Success,
            json{
                {"serial_number", serial_number},
                {"user_id", user_id}
            }
        };
    }

    util::Result Bind::unbind(const std::string &serial_number, const std::string &user_id)
    {
        using namespace model::validator;
        using namespace model::sql::reg;
        using json = nlohmann::json;

        if (!is_vaild_serial_number(serial_number))
            return {
                util::status_code::BadRequest,
                util::error_code::Invalid_serialnumber_format,
                json{}
            };

        if (const auto a = EdgeIOSBindingRegistrar::unbind(serial_number, user_id); a == 1)
            return {
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        return util::Result{
            util::status_code::Success,
            util::error_code::Success,
            json{
                    {"serial_number", serial_number},
                    {"user_id", user_id}
            }
        };
    }
}
