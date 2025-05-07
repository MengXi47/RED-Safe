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

#include "controller.hpp"
#include "service/service.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>

using json = nlohmann::json;
using Handler = std::function<json(const json&)>;

namespace redsafe::apiserver
{
    Controller::Controller(http::request<http::string_body> req)
        : req_(req)
    {
    }

    response Controller::handle_request()
    {
        json req_body;
        try
        {
            req_body = json::parse(req_.body());
        }
        catch (const nlohmann::json::parse_error& e)
        {
            return make_error_response(400, "Invalid JSON");
        }

        if (!req_body.contains("message_type"))
            return make_error_response(400, "Missing message_type");
        auto message_type = req_body.value("message_type", std::string{});

        static const std::unordered_map<std::string, Handler> handlers =
        {
            {"register", [this](const json& body)
                {
                    return std::make_shared<service::RegistrationService>(
                       body.value("serial_number", std::string{}))->registerUser();
                }
            },
            {"test", [this](const json& body)
                {
                    return json{{"response", "Hello from RED-Safe!"}};
                }
            }
        };

        json ResponseBody;

        auto it = handlers.find(message_type);
        if (it != handlers.end())
            ResponseBody = it->second(req_body);
        else 
            return make_error_response(400, "Unknown message_type");

        response res{ http::status::ok, req_.version() };
        res.set(http::field::server, "RED-Safe");
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req_.keep_alive());
        res.body() = ResponseBody.dump();
        res.prepare_payload();

        return res;
    }

    response Controller::make_error_response(int status_code, const std::string &message)
    {
        json j = { {"error", message} };
        response res{ static_cast<http::status>(status_code), 11 };
        res.set(http::field::server, "RED-Safe");
        res.set(http::field::content_type, "application/json");
        res.body() = j.dump();
        res.prepare_payload();
        return res;
    }
}
