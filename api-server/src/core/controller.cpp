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

#include "../util/timestamp.hpp"
#include "controller.hpp"
#include "../service/service.hpp"

#include <unordered_map>
#include <functional>

namespace redsafe::apiserver
{
    Controller::Controller(http::request<http::string_body> req)
        : req_(std::move(req))
    {
    }

    response Controller::handle_request()
    {
        static const std::unordered_map<std::string, std::function<json(const json&)>> handlers =
        {
            {"/edge/register", [this](const json& body)
            {
                try
                {
                    if (!body.contains("serial_number") || !body.contains("version"))
                        throw std::invalid_argument("Missing serial_number or version");

                    return std::make_shared<service::EdgeDeviceRegistrationService>(
                        body.value("version", std::string{}),
                        body.value("serial_number", std::string{}))->Register();
                }
                catch (const std::runtime_error& e)
                {
                    if (std::string(e.what()) == "Edge device already registered")
                        return json{{"status", "error"}, {"message", e.what()}, {"code", 409}};
                    throw;
                }
                catch (const std::invalid_argument& e)
                {
                    return json{{"status", "error"}, {"message", e.what()}, {"code", 400}};
                }
                catch ([[maybe_unused]] std::exception& e)
                {
                    return json{{"status", "error"}, {"message", "Internal server error"}, {"code", 500}};
                }
            }},

            {"/user/register", [this](const json& body)
            {
                try
                {
                    if (!body.contains("email") || !body.contains("user_name") || !body.contains("password"))
                        throw std::invalid_argument("Missing email or user_name or password");

                    return std::make_shared<service::UserRegistrationService>(
                        body.value("email", std::string{}),
                        body.value("user_name", std::string{}),
                        body.value("password", std::string{}))->Register();
                }
                catch (const std::runtime_error& e)
                {
                    if (std::string(e.what()) == "Email already registered")
                        return json{{"status", "error"}, {"message", e.what()}, {"code", 409}};
                    throw;
                }
                catch (const std::invalid_argument& e)
                {
                    return json{{"status", "error"}, {"message", e.what()}, {"code", 400}};
                }
                catch ([[maybe_unused]] const std::exception& e)
                {
                    return json{{"status", "error"}, {"message", "Internal server error"}, {"code", 500}};
                }
            }},

            {"/user/login", [this](const json& body)
            {
                try
                {
                    if (!body.contains("email") || !body.contains("password"))
                        throw std::invalid_argument("Missing email or password");

                    return std::make_shared<service::UserLoginLogoutService>(
                        body.value("email", std::string{}),
                        body.value("password", std::string{}))->login();
                }
                catch (const std::runtime_error& e)
                {
                    if (std::string(e.what()) == "Email or Password Error")
                        return json{{"status", "error"}, {"message", e.what()}, {"code", 400}};
                    throw;
                }
                catch (const std::invalid_argument& e)
                {
                    return json{{"status", "error"}, {"message", e.what()}, {"code", 400}};
                }
                catch ([[maybe_unused]] const std::exception& e)
                {
                    return json{{"status", "error"}, {"message", "Internal server error"}, {"code", 500}};
                }
            }},

            {"/ios/register", [this](const json& body)
            {
                try
                {
                    if (!body.contains("user_id") || !body.contains("apns_token"))
                        throw std::invalid_argument("Missing user_id or apns_token");

                    return std::make_shared<service::IOSDeviceRegistrationService>(
                        body.value("ios_device_id", std::string{}),
                        body.value("user_id", std::string{}),
                        body.value("apns_token", std::string{}),
                        body.value("device_name", std::string{}))->Register();
                }
                catch (const std::invalid_argument& e)
                {
                    return json{{"status", "error"}, {"message", e.what()}, {"code", 400}};
                }
                catch ([[maybe_unused]] const std::exception& e)
                {
                    return json{{"status", "error"}, {"message", "Internal server error"}, {"code", 500}};
                }
            }},

            {"/ios/bind", [this](const json& body)
            {
                try
                {
                    if (!body.contains("user_id") || !body.contains("serial_number"))
                        throw std::invalid_argument("Missing user_id or serial_number");

                    return std::make_shared<service::IOSDeviceBindService>(
                        body.value("serial_number", std::string{}),
                        body.value("user_id", std::string{}))->bind();
                }
                catch (const std::runtime_error& e)
                {
                    if (std::string(e.what()) == "Binding already exists")
                        return json{{"status", "error"}, {"message", e.what()}, {"code", 409}};
                    throw;
                }
                catch (const std::invalid_argument& e)
                {
                    return json{{"status", "error"}, {"message", e.what()}, {"code", 400}};
                }
                catch ([[maybe_unused]] const std::exception& e)
                {
                    return json{{"status", "error"}, {"message", "Internal server error"}, {"code", 500}};
                }
            }},

            {"/ios/unbind", [this](const json& body)
            {
                try
                {
                    if (!body.contains("ios_device_id") || !body.contains("serial_number"))
                        throw std::invalid_argument("Missing ios_device_id or serial_number");

                    return std::make_shared<service::IOSDeviceBindService>(
                        body.value("serial_number", std::string{}),
                        body.value("ios_device_id", std::string{}))->unbind();
                }
                catch (const std::invalid_argument& e)
                {
                    return json{{"status", "error"}, {"message", e.what()}, {"code", 400}};
                }
                catch ([[maybe_unused]] const std::exception& e)
                {
                    return json{{"status", "error"}, {"message", "Internal server error"}, {"code", 500}};
                }
            }}
        };

        const auto target = req_.target();
        json ResponseBody;

        if (const auto it = handlers.find(target); it != handlers.end())
        {
            json req_body;
            try
            {
                req_body = json::parse(req_.body());
            }
            catch ([[maybe_unused]] nlohmann::json::parse_error& e)
            {
                return make_response(400, {{"status", "error"}, {"message", "Invalid JSON"}});
            }
            ResponseBody = it->second(req_body);
        }
        else
            return make_response(404, {{"status", "error"}, {"message", "Unknown endpoint"}});

        if (ResponseBody.contains("code"))
        {
            const int status_code = ResponseBody.value("code", 500);
            ResponseBody.erase("code");
            return make_response(status_code, ResponseBody);
        }
        return make_response(200, ResponseBody);
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
