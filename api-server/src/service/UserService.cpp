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

#ifndef REDSAFE_USER_SERVICE_CPP
#define REDSAFE_USER_SERVICE_CPP

#include <nlohmann/json.hpp>

#include "UserService.hpp"
#include "TokenService.hpp"
#include "../model/sql/read.hpp"
#include "../model/sql/write.hpp"
#include "../model/validator/ParameterValidation.hpp"
#include "../util/IOstream.hpp"
#include "../util/logger.hpp"

namespace redsafe::apiserver::service::User
{
    util::Result signin::start(
        const std::string &email,
        const std::string &password)
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

        if (!util::VerifyHASH(password_hash, password))
            return util::Result{
                util::status_code::BadRequest,
                util::error_code::Email_or_Password_Error,
                json{}
            };

        const auto user_name = UserNameFinder::start(email);
        if (user_name.empty())
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        const auto user_id = UserIDFinder::start(email);
        if (user_id.empty())
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };

        token::CreateAccessToken AT(user_id);
        if (AT.start() == 1)
        {
            util::cerr() << AT.getErrorMessage();
            util::log(util::LogFile::server, util::Level::ERROR) << AT.getErrorMessage();
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };
        }

        token::CreateRefreshToken RT(user_id);
        if (RT.start() == 1)
        {
            util::cerr() << RT.getErrorMessage();
            util::log(util::LogFile::server, util::Level::ERROR) << RT.getErrorMessage();
            return util::Result{
                util::status_code::InternalServerError,
                util::error_code::Internal_server_error,
                json{}
            };
        }

        return util::Result{
            util::status_code::Success,
            util::error_code::Success,
            json{
                {"user_name", user_name},
                {"access_token", AT.getAccessToken()},
                {"refresh_token", RT.getRefreshToken()}
            }
        };
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
            password_hash = util::HASH(password);
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

        return util::Result{
            util::status_code::Success,
            util::error_code::Success,
            json{
                {"email", email},
                {"user_name", user_name}
            }
        };
    }

    util::Result Binding::bind(const std::string &serial_number, const std::string &user_id)
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

    util::Result Binding::unbind(const std::string &serial_number, const std::string &user_id)
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

#endif