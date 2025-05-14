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

#include "util.hpp"

#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace redsafe::apiserver
{
    namespace beast      = boost::beast;
    namespace http       = beast::http;
    using     tcp        = boost::asio::ip::tcp;

    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(std::shared_ptr<tcp::socket> sock);
        void start();
    private:
        void do_read();
        void handle_request();
        template<class Response>
        void do_write(Response&& res);

        std::shared_ptr<tcp::socket>        socket_;
        beast::flat_buffer                  buffer_;
        http::request<http::string_body>    req_;
        http::response<http::string_body>   res_;
    };
}