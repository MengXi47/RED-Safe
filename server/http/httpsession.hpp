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

#pragma once

#include <memory>
#include <functional>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace redsafe::server {
class Session : public std::enable_shared_from_this<Session> {
 public:
  using RequestHandler = std::function<
      boost::beast::http::response<boost::beast::http::string_body>(
          const boost::beast::http::request<boost::beast::http::string_body>&)>;

  Session(boost::asio::ip::tcp::socket sock, RequestHandler handler);
  void start();

 private:
  boost::asio::awaitable<void> run();
  boost::asio::ip::tcp::socket socket_;
  boost::beast::flat_buffer buffer_;
  boost::beast::http::request<boost::beast::http::string_body> req_;
  RequestHandler handler_;
};
} // namespace redsafe::server
