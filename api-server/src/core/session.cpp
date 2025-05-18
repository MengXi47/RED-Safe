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
#include "../util/logger.hpp"

#include <iostream>

namespace beast      = boost::beast;
namespace http       = beast::http;
using     tcp        = boost::asio::ip::tcp;

namespace redsafe::apiserver
{
    Session::Session(std::shared_ptr<tcp::socket> sock)
        : socket_(std::move(sock))
    {
    }

    void Session::start()
    {
        do_read();
    }

    void Session::do_read()
    {
        auto self = shared_from_this();
        req_ = {};
        buffer_.consume(buffer_.size());
        http::async_read(
            *socket_,
            buffer_,
            req_,
            [this, self](const boost::system::error_code& ec, std::size_t)
            {
                if (ec)
                {
                    if(ec == http::error::end_of_stream || ec == boost::asio::error::connection_reset)
                        return;
                    util::log(util::LogFile::server, util::Level::ERROR)
                        << "Read failure: " << ec.message();
                    std::cerr << "Read failure: " << ec.message() << '\n';
                    return;
                }
                util::log(util::LogFile::access, util::Level::INFO)
                    << req_.base()["X-Real-IP"]
                    << " " << req_;
                self->handle_request();
            }
        );
    }

    void Session::handle_request()
    {
        do_write(std::move(std::make_shared<Controller>(req_)->handle_request()));
    }

    template<class Response>
    void Session::do_write(Response&& res)
    {
        auto self = shared_from_this();
        auto sp = std::make_shared<std::decay_t<Response>>(std::forward<Response>(res));
        http::async_write(
            *socket_,
            *sp,
            [this, self, sp]([[maybe_unused]] const auto ec, std::size_t)
            {
                if (ec)
                {
                    if(ec == boost::asio::error::broken_pipe)
                        return;
                    util::log(util::LogFile::server, util::Level::ERROR)
                        << " Write failure: " << ec.message();
                    std::cerr << "Write failure: " << ec.message() << '\n';
                    return;
                }
                self->do_read();
            }
        );
    }
}
