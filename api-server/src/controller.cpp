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

#include <iostream>

namespace redsafe::apiserver
{
    Controller::Controller(const http::request<http::string_body> req)
        : req_(req)
    {
    }

    response Controller::handle_request()
    {
        response res{ http::status::ok, req_.version() };
        res.set(http::field::server, "RED-Safe");
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req_.keep_alive());
        res.body() = str_response;
        res.prepare_payload();

        return res;
    }
}