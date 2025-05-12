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

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace redsafe::apiserver
{
    namespace beast = boost::beast;
    namespace http  = beast::http;
    using response = http::response<http::string_body>;

    class Controller
    {
    public:
        explicit Controller(http::request<http::string_body> req);
        response handle_request();
    private:
        static response make_error_response(int status_code, const std::string& message);
        std::string str_response = "ttttttttttttttttttt";
        http::request<http::string_body> req_;
    };
}