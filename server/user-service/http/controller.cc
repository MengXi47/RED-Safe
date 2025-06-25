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

#include "controller.hpp"

#include <functional>
#include <thread>
#include <folly/container/F14Map.h>

#include "../service/TokenService.hpp"
#include "../service/UserService.hpp"
#include "../util/logger.hpp"
#include "../util/response.hpp"

namespace redsafe::server {
Controller::Controller(http::request<http::string_body> req)
    : req_(std::move(req)) {}

response Controller::handle_request() const {
  static const folly::F14FastMap<
      std::string,
      std::function<util::Result(const http::request<http::string_body>&)>>
      POST_map = {
          {"/user/signup",
           [](const http::request<http::string_body>& req) {
             try {
               const json body = json::parse(req.body());
               if (!body.contains("email") || !body.contains("user_name") ||
                   !body.contains("password")) {
                 return util::Result{
                     util::status_code::BadRequest,
                     util::error_code::Missing_email_or_user_name_or_password,
                     json{}};
               }

               return service::User::signup::run(
                   body.value("email", std::string{}),
                   body.value("user_name", std::string{}),
                   body.value("password", std::string{}));
             } catch ([[maybe_unused]] nlohmann::json::parse_error& e) {
               return util::Result{
                   util::status_code::BadRequest,
                   util::error_code::Invalid_JSON,
                   json{}};
             }
           }},

          {"/user/signin",
           [](const http::request<http::string_body>& req) {
             try {
               const json body = json::parse(req.body());

               if (!body.contains("email") || !body.contains("password")) {
                 return util::Result{
                     util::status_code::BadRequest,
                     util::error_code::Missing_email_or_password,
                     json{}};
               }

               return service::User::signin::run(
                   body.value("email", std::string{}),
                   body.value("password", std::string{}));
             } catch ([[maybe_unused]] nlohmann::json::parse_error& e) {
               return util::Result{
                   util::status_code::BadRequest,
                   util::error_code::Invalid_JSON,
                   json{}};
             }
           }},

          {"/auth/refresh",
             [](const http::request<http::string_body>& req) {
               const auto refresh_token = get_refresh_token(req);

               if (refresh_token.empty()) {
                 return util::Result{
                     util::status_code::BadRequest,
                     util::error_code::Missing_refresh_token,
                     json{}};
               }

               return service::token::CheckAndRefreshRefreshToken::run(
                   refresh_token);
             }},

          {"/auth/out",
           [](const http::request<http::string_body>& req) {
             const auto refresh_token = get_refresh_token(req);

             if (refresh_token.empty()) {
               return util::Result{
                   util::status_code::BadRequest,
                   util::error_code::Missing_refresh_token,
                   json{}};
             }

             return service::token::RevokeRefreshToken::run(refresh_token);
           }},
      };

  static const folly::F14FastMap<
      std::string,
      std::function<util::Result(const http::request<http::string_body>&)>>
      GET_map = {
          {"/user/all",
           [this](const http::request<http::string_body>& req) {
             const auto access_token = get_access_token(req);

             if (access_token.empty()) {
               return util::Result{
                   util::status_code::BadRequest,
                   util::error_code::Access_Token_invalid,
                   json{}};
             }

             return service::User::GetUserInformation::run(access_token);
           }},
      };

  util::Result ResponseBody;

  if (req_.method() == http::verb::get) {
    if (const auto it = GET_map.find(req_.target()); it != GET_map.end()) {
      ResponseBody = it->second(req_);
    } else {
      return make_response(
          static_cast<int>(util::status_code::NotFound),
          json{
              {"error_code",
               static_cast<int>(util::error_code::Unknown_endpoint)}});
    }
  } else if (req_.method() == http::verb::post) {
    if (const auto it = POST_map.find(req_.target()); it != POST_map.end()) {
      ResponseBody = it->second(req_);
    } else {
      return make_response(
          static_cast<int>(util::status_code::NotFound),
          json{
              {"error_code",
               static_cast<int>(util::error_code::Unknown_endpoint)}});
    }
  } else {
    return make_response(
        static_cast<int>(util::status_code::NotFound),
        json{
            {"error_code",
             static_cast<int>(util::error_code::Unknown_endpoint)}});
  }

  /// 組合 response
  ResponseBody.body["error_code"] = static_cast<int>(ResponseBody.ec);
  if (ResponseBody.token.empty()) {
    return make_response(static_cast<int>(ResponseBody.sc), ResponseBody.body);
  }
  return make_response(
      static_cast<int>(ResponseBody.sc),
      ResponseBody.body,
      "refresh_token=" + ResponseBody.token + // 明文 RT
          "; Path=/auth" // 只有 /auth/* 會帶
          "; Max-Age=2592000" // 30 天（秒）
          "; HttpOnly; Secure; SameSite=Strict");
}

response Controller::make_response(int status_code, const json& j) {
  response res{static_cast<http::status>(status_code), 11};
  res.keep_alive(true);
  res.set(http::field::server, "RED-Safe");
  res.set(http::field::content_type, "application/json");
  res.body() = j.dump();
  res.prepare_payload();
  return res;
}

response Controller::make_response(
    int status_code, const json& j, const std::string_view cookie) {
  response res{static_cast<http::status>(status_code), 11};
  res.keep_alive(true);
  res.set(http::field::server, "RED-Safe");
  res.set(http::field::content_type, "application/json");
  if (!cookie.empty()) {
    res.set(http::field::set_cookie, cookie);
  }
  res.body() = j.dump();
  res.prepare_payload();
  return res;
}

std::string Controller::get_access_token(
    const http::request<http::string_body>& req) {
  std::string accesstoken;
  if (req.base().count("Authorization")) {
    const auto authHeader = std::string(req.base().at("Authorization"));
    if (constexpr std::string_view prefix = "Bearer ";
        authHeader.size() > prefix.size() &&
        authHeader.substr(0, prefix.size()) == prefix) {
      accesstoken = authHeader.substr(prefix.size());
      const auto l = accesstoken.find_first_not_of(" \t\r\n");
      const auto r = accesstoken.find_last_not_of(" \t\r\n");
      if (l != std::string::npos) {
        accesstoken = accesstoken.substr(l, r - l + 1);
      } else {
        accesstoken.clear();
      }
    }
  }
  return accesstoken;
}

std::string Controller::get_refresh_token(
    const http::request<http::string_body>& req) {
  std::string refreshtoken;
  if (req.base().count("Cookie")) {
    const auto cookieHeader = std::string(req.base().at("Cookie"));
    static const std::string key = "refresh_token=";
    if (auto pos = cookieHeader.find(key); pos != std::string::npos) {
      pos += key.size();
      if (const auto end = cookieHeader.find(';', pos);
          end == std::string::npos) {
        refreshtoken = cookieHeader.substr(pos);
      } else {
        refreshtoken = cookieHeader.substr(pos, end - pos);
      }
      static constexpr char ws[] = " \t\r\n";
      const auto l = refreshtoken.find_first_not_of(ws);
      const auto r = refreshtoken.find_last_not_of(ws);
      if (l != std::string::npos) {
        refreshtoken = refreshtoken.substr(l, r - l + 1);
      } else {
        refreshtoken.clear();
      }
    }
  }
  // 嚴格認證 Refresh Token 格式：64 字元的小寫十六進位
  if (refreshtoken.size() != 64 ||
      refreshtoken.find_first_not_of("0123456789abcdef") != std::string::npos) {
    refreshtoken.clear();
  }
  return refreshtoken;
}
} // namespace redsafe::server