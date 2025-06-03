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

#include <unordered_map>
#include <functional>

#include "controller.hpp"
#include "../service/EdgeService.hpp"
#include "../service/IOSAPPService.hpp"
#include "../service/UserService.hpp"
#include "../service/TokenService.hpp"
#include "../util/logger.hpp"
#include "../util/response.hpp"

namespace redsafe::apiserver
{
    Controller::Controller(http::request<http::string_body> req)
        : req_(std::move(req))
    {
    }

    response Controller::handle_request()
    {
        static const std::unordered_map<
            std::string,
            std::function<util::Result(const json&, const std::string&)>> handlers =
        {
            {"/edge/signup", [this](const json& body, const std::string&)
            {
                if (!body.contains("serial_number") || !body.contains("version"))
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_serial_number_or_version,
                        json{}
                    };

                return service::edge::Register::start(
                    body.value("version", std::string{}),
                    body.value("serial_number", std::string{})
                );
            }},

            {"/user/signup", [this](const json& body, const std::string&)
            {
                if (!body.contains("email")     ||
                    !body.contains("user_name") ||
                    !body.contains("password"))
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_email_or_user_name_or_password,
                        json{}
                    };

                return service::User::signup::start(
                    body.value("email", std::string{}),
                    body.value("user_name", std::string{}),
                    body.value("password", std::string{}));
            }},

            {"/user/signin", [this](const json& body, const std::string&)
            {
                if (!body.contains("email") || !body.contains("password"))
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_email_or_password,
                        json{}
                    };

                return service::User::signin::start(
                    body.value("email", std::string{}),
                    body.value("password", std::string{}));
            }},

            {"/ios/signup", [this](const json& body, const std::string&)
            {
                if (!body.contains("user_id") || !body.contains("apns_token"))
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_user_id_or_apns_token,
                        json{}
                    };

                return service::IOSAPP::IOSAPPService::start(
                    body.value("ios_device_id", std::string{}),
                    body.value("user_id", std::string{}),
                    body.value("apns_token", std::string{}),
                    body.value("device_name", std::string{})
                );
            }},

            {"/ios/bind", [this](const json& body, const std::string&)
            {
                if (!body.contains("user_id") || !body.contains("serial_number"))
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_user_id_or_serial_number,
                        json{}
                    };

                return service::User::Binding::bind(
                    body.value("serial_number", std::string{}),
                    body.value("user_id", std::string{})
                );
            }},

            {"/ios/unbind", [this](const json& body, const std::string&)
            {
                if (!body.contains("user_id") || !body.contains("serial_number"))
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_user_id_or_serial_number,
                        json{}
                    };

                return service::User::Binding::unbind(
                    body.value("serial_number", std::string{}),
                    body.value("user_id", std::string{})
                );
            }},

            {"/auth/refresh", [this](const json&, const std::string& refreshtoken)
            {
                if (refreshtoken.empty())
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_refresh_token,
                        json{}
                    };

                return service::token::CheckAndRefreshRefreshToken::run(refreshtoken);
            }}
        };

        const auto target = req_.target();
        util::Result ResponseBody;

        if (const auto it = handlers.find(target); it != handlers.end())
        {
            json req_body;
            try
            {
                req_body = json::parse(req_.body());
            }
            catch ([[maybe_unused]] nlohmann::json::parse_error& e)
            {
                return make_response(
                    static_cast<int>(util::status_code::BadRequest),
                    json{{"error_code", static_cast<int>(util::error_code::Invalid_JSON)}});
            }

            /// 從 Authorization 標頭取 token（若有）
            std::string authToken;
            if (req_.base().count("Authorization"))
            {
                const auto authHeader = std::string(req_.base().at("Authorization"));
                if (constexpr std::string_view prefix = "Bearer ";
                    authHeader.size() > prefix.size() &&
                    authHeader.substr(0, prefix.size())
                    == prefix)
                {
                    authToken = authHeader.substr(prefix.size());
                    // trim whitespace (可選)
                    const auto l = authToken.find_first_not_of(" \t");
                    const auto r = authToken.find_last_not_of(" \t");
                    if (l != std::string::npos)
                        authToken = authToken.substr(l, r - l + 1);
                    else
                        authToken.clear();
                }
            }
            ResponseBody = it->second(req_body, authToken);
        }
        else
            return make_response(
                static_cast<int>(util::status_code::NotFound),
                json{{"error_code", static_cast<int>(util::error_code::Unknown_endpoint)}});

        ResponseBody.body["error_code"] = static_cast<int>(ResponseBody.ec);
        return make_response(static_cast<int>(ResponseBody.sc), ResponseBody.body);
    }

    response Controller::make_response(int status_code, const json& j)
    {
        response res{ static_cast<http::status>(status_code), 11 };
        res.keep_alive(true);
        res.set(http::field::server, "RED-Safe");
        res.set(http::field::content_type, "application/json");
        res.body() = j.dump();
        res.prepare_payload();
        return res;
    }
}
