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

#include <nlohmann/json.hpp>
#include <iostream>
#include <regex>
#include <sodium.h>
#include <vector>

#include "UserRegistrationService.hpp"
#include "../model/sql/RegistrarModels.hpp"
#include "../model/sql/FinderModels.hpp"
#include "../model/validator/ParameterValidation.hpp"
#include "../util/IOstream.hpp"
#include "../util/logger.hpp"

namespace redsafe::apiserver::service
{
    UserRegistrationService::UserRegistrationService(std::string email,
                                                     std::string user_name,
                                                     std::string password)
        : email_    (std::move(email)),
          user_name_(std::move(user_name)),
          password_ (std::move(password))
    {
    }

    util::Result UserRegistrationService::start() const
    {
        using namespace model::validator;
        using namespace model::sql::reg;
        using namespace model::sql::fin;
        using json = nlohmann::json;

        if (!is_vaild_email(email_))
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

        if (!is_vaild_password(password_))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Invalid_password_format,
                json{}
            };

        std::string password_hash;
        try
        {
            password_hash = PasswordHASH();
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

        if (const auto e = UserRegistrar::start(email_, user_name_, password_hash); e == 1)
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

        if (const auto user_id = UserIDFinder::start(email_); user_id.empty())
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
                        {"email", email_},
                        {"user_name", user_name_},
                        {"user_id", user_id}
                }
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
                crypto_pwhash_MEMLIMIT_INTERACTIVE
            ) != 0)
            throw std::runtime_error("Password hashing failed");
        return {hashBuf.data()};
    }
}