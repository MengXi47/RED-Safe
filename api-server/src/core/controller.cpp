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
#include "../util/IOstream.hpp"
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
            std::function<util::Result(const http::request<http::string_body>&)>> handlers =
        {
            {"/edge/signup", [this](const http::request<http::string_body>& req)
            {
                try
                {
                    const json body = json::parse(req.body());
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
                }
                catch ([[maybe_unused]] nlohmann::json::parse_error& e)
                {
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Invalid_JSON,
                        json{}
                    };
                }


            }},

            {"/user/signup", [this](const http::request<http::string_body>& req)
            {
                try
                {
                    const json body = json::parse(req.body());
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
                }
                catch ([[maybe_unused]] nlohmann::json::parse_error& e)
                {
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Invalid_JSON,
                        json{}
                    };
                }
            }},

            {"/user/signin", [this](const http::request<http::string_body>& req)
            {
                try
                {
                    const json body = json::parse(req.body());

                    if (!body.contains("email") || !body.contains("password"))
                        return util::Result{
                            util::status_code::BadRequest,
                            util::error_code::Missing_email_or_password,
                            json{}
                        };

                    return service::User::signin::start(
                        body.value("email", std::string{}),
                        body.value("password", std::string{}));
                }
                catch ([[maybe_unused]] nlohmann::json::parse_error& e)
                {
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Invalid_JSON,
                        json{}
                    };
                }
            }},

            {"/ios/signup", [this](const http::request<http::string_body>& req)
            {
                try
                {
                    const json body = json::parse(req.body());
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
                }
                catch ([[maybe_unused]] nlohmann::json::parse_error& e)
                {
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Invalid_JSON,
                        json{}
                    };
                }
            }},

            {"/ios/bind", [this](const http::request<http::string_body>& req)
            {
                try
                {
                    const json body = json::parse(req.body());
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
                }
                catch ([[maybe_unused]] nlohmann::json::parse_error& e)
                {
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Invalid_JSON,
                        json{}
                    };
                }
            }},

            {"/ios/unbind", [this](const http::request<http::string_body>& req)
            {
                try
                {
                    const json body = json::parse(req.body());
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
                }
                catch ([[maybe_unused]] nlohmann::json::parse_error& e)
                {
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Invalid_JSON,
                        json{}
                    };
                }
            }},

            {"/auth/refresh", [this](const http::request<http::string_body>& req)
            {
                std::string refreshtoken;
                if (req.base().count("Authorization"))
                {
                    const auto authHeader = std::string(req.base().at("Authorization"));
                    if (constexpr std::string_view prefix = "Bearer ";
                        authHeader.size() > prefix.size() &&
                        authHeader.substr(0, prefix.size())
                        == prefix)
                    {
                        refreshtoken = authHeader.substr(prefix.size());
                        const auto l = refreshtoken.find_first_not_of(" \t\r\n");
                        const auto r = refreshtoken.find_last_not_of(" \t\r\n");
                        if (l != std::string::npos)
                            refreshtoken = refreshtoken.substr(l, r - l + 1);
                        else
                            refreshtoken.clear();
                    }
                    /// 嚴格認證 Refresh Token 64byteHEX
                    if (refreshtoken.size() != 64 ||
                        refreshtoken.find_first_not_of("0123456789abcdef")
                        != std::string::npos)
                        refreshtoken.clear();
                }

                if (refreshtoken.empty())
                    return util::Result{
                        util::status_code::BadRequest,
                        util::error_code::Missing_refresh_token,
                        json{}
                    };

                return service::token::CheckAndRefreshRefreshToken::run(refreshtoken);
                // std::string refreshtoken;
                // if (req.base().count("Cookie"))
                // {
                //     static const auto cookieHeader = std::string(req.base().at("Cookie"));
                //     static const std::string key = "refresh_token=";
                //     if (auto pos = cookieHeader.find(key); pos != std::string::npos)
                //     {
                //         pos += key.size();
                //         const auto end = cookieHeader.find(';', pos);
                //         if (end == std::string::npos)
                //             refreshtoken = cookieHeader.substr(pos);
                //         else
                //             refreshtoken = cookieHeader.substr(pos, end - pos);
                //         static constexpr char ws[] = " \t\r\n";
                //         const auto l = refreshtoken.find_first_not_of(ws);
                //         const auto r = refreshtoken.find_last_not_of(ws);
                //         if (l != std::string::npos)
                //             refreshtoken = refreshtoken.substr(l, r - l + 1);
                //         else
                //             refreshtoken.clear();
                //     }
                // }
                // // 嚴格認證 Refresh Token 格式：64 字元的小寫十六進位
                // if (refreshtoken.size() != 64 ||
                //     refreshtoken.find_first_not_of("0123456789abcdef")
                //     != std::string::npos)
                //     refreshtoken.clear();
                //
                // if (refreshtoken.empty())
                //     return util::Result{
                //         util::status_code::BadRequest,
                //         util::error_code::Missing_refresh_token,
                //         json{}
                //     };
                //
                // return service::token::CheckAndRefreshRefreshToken::run(refreshtoken);
            }}
        };

        util::Result ResponseBody;

        if (const auto it = handlers.find(req_.target()); it != handlers.end())
            ResponseBody = it->second(req_);
        else
            return make_response(
                static_cast<int>(util::status_code::NotFound),
                json{{"error_code", static_cast<int>(util::error_code::Unknown_endpoint)}});

        ResponseBody.body["error_code"] = static_cast<int>(ResponseBody.ec);
        if (ResponseBody.token.empty())
            return make_response(static_cast<int>(ResponseBody.sc), ResponseBody.body);
        return make_response(static_cast<int>(ResponseBody.sc), ResponseBody.body,
                             "refresh_token=" + ResponseBody.token + // 明文 RT
                             "; Path=/auth" // 只有 /auth/* 會帶
                             "; Max-Age=2592000" // 30 天（秒）
                             "; HttpOnly; Secure; SameSite=Strict");

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

    response Controller::make_response(int status_code,
                                       const json& j,
                                       const std::string_view cookie)
    {
        response res{ static_cast<http::status>(status_code), 11 };
        res.keep_alive(true);
        res.set(http::field::server, "RED-Safe");
        res.set(http::field::content_type, "application/json");
        if (!cookie.empty())
            res.set(http::field::set_cookie, cookie);
        res.body() = j.dump();
        res.prepare_payload();
        return res;
    }
}
