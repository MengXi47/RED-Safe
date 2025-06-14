/******************************************************************************
   Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights
Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly
prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source
code;
     3. Distribute, display, or otherwise use this source code or its
derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
******************************************************************************/

#include "httpsession.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/http.hpp>

#include "../util/IOstream.hpp"
#include "../util/logger.hpp"
#include "controller.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

namespace redsafe::server {
Session::Session(tcp::socket sock) : socket_(std::move(sock)) {}

void Session::start() {
  boost::asio::co_spawn(
      socket_.get_executor(),
      [self = shared_from_this()]() { return self->run(); },
      boost::asio::detached);
}

boost::asio::awaitable<void> Session::run() {
  try {
    for (;;) {
      req_ = {};
      buffer_.consume(buffer_.size());
      co_await http::async_read(
          socket_, buffer_, req_, boost::asio::use_awaitable);
      util::cout() << util::current_timestamp() << req_.base()["X-Real-IP"]
                   << " " << req_.method() << " " << req_.target() << " "
                   << req_.body() << '\n';
      util::log(util::LogFile::access, util::Level::INFO)
          << req_.base()["X-Real-IP"] << " " << req_.method() << " "
          << req_.target() << " " << req_.body();
      auto response = std::make_shared<Controller>(req_)->handle_request();
      co_await http::async_write(socket_, response, boost::asio::use_awaitable);
    }
  } catch (const std::exception& e) {
#ifndef NDEBUG
    util::cout() << e.what() << '\n';
#endif
    util::cout() << util::current_timestamp() << "nginx disconnection: "
                 << socket_.remote_endpoint().address().to_string() << ':'
                 << socket_.remote_endpoint().port() << '\n';
    socket_.close();
  }
}
} // namespace redsafe::apiserver
