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
*******************************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <boost/beast/http.hpp>

namespace redsafe::server {
class Server {
 public:
  using RequestHandler = std::function<
      boost::beast::http::response<boost::beast::http::string_body>(
          const boost::beast::http::request<boost::beast::http::string_body>&)>;

  Server(uint16_t port, RequestHandler handler);
  ~Server();

  void start() const;
  void stop() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace redsafe::server
