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

#include "session.hpp"
#include "controller.hpp"

#include <iostream>

namespace redsafe::apiserver
{
    Session::Session(std::shared_ptr<ssl_stream> socket)
        : socket_(std::move(socket))
    {
    }

    void Session::start()
    {
        auto self = shared_from_this();
        socket_->async_handshake(
            boost::asio::ssl::stream_base::server,
            [self](const boost::system::error_code &ec)
            {
                if(ec)
                {
                    std::cerr << "Handshake failed: " << ec.message() << "\n";
                    return;
                }
                self->do_read();
            }
        );
    }

    void Session::do_read()
    {
        auto self = shared_from_this();
        http::async_read(
            *socket_,
            buffer_,
            req_,
            [self](const boost::system::error_code &ec, std::size_t)
            {
                if(ec) {
                    std::cerr << "Read failure: " << ec.message() << "\n";
                    return;
                }
                self->handle_request();
            }
        );
    }

    void Session::handle_request()
    {
        http::response<http::string_body> res =
            std::make_shared<redsafe::apiserver::Controller>(req_)->handle_request();

        do_write(std::move(res));
    }

    template<class Response>
    void Session::do_write(Response&& res)
    {
        auto self = shared_from_this();
        auto sp = std::make_shared<std::decay_t<Response>>(std::forward<Response>(res));

        http::async_write(
            *socket_,
            *sp,
            [self, sp](const boost::system::error_code &ec, std::size_t)
            {
                self->socket_->async_shutdown([self](boost::system::error_code) {});
            }
        );
    }
}
