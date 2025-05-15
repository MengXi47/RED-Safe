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

#include "util.hpp"
#include "controller.hpp"
#include "service/service.hpp"

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


                    return std::make_shared<service::EdgeRegistrationService>(
                        body.value("version", std::string{}),
                        body.value("serial_number", std::string{}),
                        util::current_timestamp(true))->start();
                }
                catch (const std::invalid_argument& e)
                {
                    return json{{"error", e.what()}, {"code", 400}};
                }
                catch (const std::exception& e)
                {
                    return json{{"error", e.what()}, {"code", 500}};
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
                return make_response(400, {{"error", "Invalid JSON"}});
            }
            ResponseBody = it->second(req_body);
        }
        else
            return make_response(404, {{"error", "Unknown Unknown endpoint"}});

        if (ResponseBody.contains("error"))
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
        res.set(http::field::server, "RED-Safe");
        res.set(http::field::content_type, "application/json");
        res.body() = j.dump();
        res.prepare_payload();
        return res;
    }
}
