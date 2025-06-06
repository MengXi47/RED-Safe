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

#include "controller.hpp"

#include <folly/String.h>
#include <folly/container/F14Map.h>

#include <functional>

#include "../service/EdgeService.hpp"
#include "../service/IOSAPPService.hpp"
#include "../service/TokenService.hpp"
#include "../service/UserService.hpp"
#include "../util/logger.hpp"
#include "../util/response.hpp"

namespace redsafe::apiserver {
// 建構控制器並保存請求資料
Controller::Controller(HTTPRequest req) : req_(std::move(req)) {}

// 根據請求路徑與方法呼叫服務
folly::coro::Task<HTTPResponse> Controller::handle_request() const {
  static const folly::F14FastMap<folly::StringPiece,
                                 std::function<util::Result(const HTTPRequest&)>>
      POST_map = {
          {"/edge/signup",
           [](const HTTPRequest& req) {
             try {
               const json body = parse_json(req.body);  // 解析 JSON
               if (!contains(body, "serial_number") || !contains(body, "version"))
                 return util::Result{util::status_code::BadRequest,
                                     util::error_code::Missing_serial_number_or_version, json{}};

               return service::edge::Register::start(get_string(body, "version"),
                                                     get_string(body, "serial_number"));
             } catch (const std::exception&) {
               return util::Result{util::status_code::BadRequest, util::error_code::Invalid_JSON,
                                   json{}};
             }
           }},

          {"/user/signup",
           [](const HTTPRequest& req) {
             try {
               const json body = parse_json(req.body);
               if (!contains(body, "email") || !contains(body, "user_name") ||
                   !contains(body, "password"))
                 return util::Result{util::status_code::BadRequest,
                                     util::error_code::Missing_email_or_user_name_or_password,
                                     json{}};

               return service::User::signup::run(get_string(body, "email"),
                                                 get_string(body, "user_name"),
                                                 get_string(body, "password"));
             } catch (const std::exception&) {
               return util::Result{util::status_code::BadRequest, util::error_code::Invalid_JSON,
                                   json{}};
             }
           }},

          {"/user/signin",
           [](const HTTPRequest& req) {
             try {
               const json body = parse_json(req.body);

               if (!contains(body, "email") || !contains(body, "password"))
                 return util::Result{util::status_code::BadRequest,
                                     util::error_code::Missing_email_or_password, json{}};

               return service::User::signin::run(get_string(body, "email"),
                                                 get_string(body, "password"));
             } catch (const std::exception&) {
               return util::Result{util::status_code::BadRequest, util::error_code::Invalid_JSON,
                                   json{}};
             }
           }},

          {"/ios/signup",
           [](const HTTPRequest& req) {
             try {
               const json body = parse_json(req.body);
               if (!contains(body, "user_id") || !contains(body, "apns_token"))
                 return util::Result{util::status_code::BadRequest,
                                     util::error_code::Missing_user_id_or_apns_token, json{}};

               return service::IOSAPP::IOSAPPService::start(
                   get_string(body, "ios_device_id"), get_string(body, "user_id"),
                   get_string(body, "apns_token"), get_string(body, "device_name"));
             } catch (const std::exception&) {
               return util::Result{util::status_code::BadRequest, util::error_code::Invalid_JSON,
                                   json{}};
             }
           }},

          {"/ios/bind",
           [](const HTTPRequest& req) {
             try {
               const json body = parse_json(req.body);
               if (!contains(body, "serial_number"))
                 return util::Result{util::status_code::BadRequest,
                                     util::error_code::Missing_serial_number, json{}};

               const auto access_token = get_access_token(req);

               if (access_token.empty())
                 return util::Result{
                     util::status_code::BadRequest,
                     util::error_code::Access_Token_invalid,
                 };

               return service::User::Binding::bind(get_string(body, "serial_number"), access_token);
             } catch (const std::exception&) {
               return util::Result{util::status_code::BadRequest, util::error_code::Invalid_JSON,
                                   json{}};
             }
           }},

          {"/ios/unbind",
           [](const HTTPRequest& req) {
             try {
               const json body = parse_json(req.body);
               if (!contains(body, "serial_number"))
                 return util::Result{util::status_code::BadRequest,
                                     util::error_code::Missing_serial_number, json{}};

               const auto access_token = get_access_token(req);

               if (access_token.empty())
                 return util::Result{
                     util::status_code::BadRequest,
                     util::error_code::Access_Token_invalid,
                 };

               return service::User::Binding::unbind(get_string(body, "serial_number"),
                                                     access_token);
             } catch (const std::exception&) {
               return util::Result{util::status_code::BadRequest, util::error_code::Invalid_JSON,
                                   json{}};
             }
           }},

          {"/auth/refresh",
           [](const HTTPRequest& req) {
             const auto refresh_token = get_refresh_token(req);

             if (refresh_token.empty())
               return util::Result{util::status_code::BadRequest,
                                   util::error_code::Missing_refresh_token, json{}};

             return service::token::CheckAndRefreshRefreshToken::run(refresh_token);
           }},

          {"/auth/out",
           [](const HTTPRequest& req) {
             const auto refresh_token = get_refresh_token(req);

             if (refresh_token.empty())
               return util::Result{util::status_code::BadRequest,
                                   util::error_code::Missing_refresh_token, json{}};

             return service::token::RevokeRefreshToken::run(refresh_token);
           }},
      };

  static const folly::F14FastMap<folly::StringPiece,
                                 std::function<util::Result(const HTTPRequest&)>>
      GET_map = {
          {"/user/all",
           [this](const HTTPRequest& req) {
             const auto access_token = get_access_token(req);

             if (access_token.empty())
               return util::Result{
                   util::status_code::BadRequest,
                   util::error_code::Access_Token_invalid,
               };

             return service::User::GetUserInformation::run(access_token);
           }},
      };

  util::Result ResponseBody;

  if (req_.method == "GET") {
    if (const auto it = GET_map.find(folly::StringPiece{req_.target}); it != GET_map.end())
      ResponseBody = it->second(req_);
    else
      co_return make_response(
          static_cast<int>(util::status_code::NotFound),
          json{{"error_code", static_cast<int>(util::error_code::Unknown_endpoint)}});
  } else if (req_.method == "POST") {
    if (const auto it = POST_map.find(folly::StringPiece{req_.target}); it != POST_map.end())
      ResponseBody = it->second(req_);
    else
      co_return make_response(
          static_cast<int>(util::status_code::NotFound),
          json{{"error_code", static_cast<int>(util::error_code::Unknown_endpoint)}});
  } else
    co_return make_response(
        static_cast<int>(util::status_code::NotFound),
        json{{"error_code", static_cast<int>(util::error_code::Unknown_endpoint)}});

  ResponseBody.body["error_code"] = static_cast<int>(ResponseBody.ec);
  if (ResponseBody.token.empty())
    co_return make_response(static_cast<int>(ResponseBody.sc), ResponseBody.body);
  co_return make_response(static_cast<int>(ResponseBody.sc), ResponseBody.body,
                          "refresh_token=" + ResponseBody.token +
                              "; Path=/auth"
                              "; Max-Age=2592000"
                              "; HttpOnly; Secure; SameSite=Strict");
}

// 產生不含 Cookie 的回應
HTTPResponse Controller::make_response(int status_code, const json& j) {
  HTTPResponse res;
  res.status = status_code;
  res.headers["Server"] = "RED-Safe";
  res.headers["Content-Type"] = "application/json";
  res.body = to_json(j);
  return res;
}

// 產生包含 Cookie 的回應
HTTPResponse Controller::make_response(int status_code, const json& j,
                                       const std::string_view cookie) {
  HTTPResponse res;
  res.status = status_code;
  res.headers["Server"] = "RED-Safe";
  res.headers["Content-Type"] = "application/json";
  if (!cookie.empty()) res.headers["Set-Cookie"] = std::string(cookie);
  res.body = to_json(j);
  return res;
}

// 從 Authorization 標頭取得 Access Token
std::string Controller::get_access_token(const HTTPRequest& req) {
  std::string accesstoken;
  auto it = req.headers.find("Authorization");
  if (it != req.headers.end()) {
    const auto& authHeader = it->second;
    constexpr std::string_view prefix = "Bearer ";
    if (authHeader.size() > prefix.size() && authHeader.substr(0, prefix.size()) == prefix) {
      accesstoken = authHeader.substr(prefix.size());
      const auto l = accesstoken.find_first_not_of(" \t\r\n");
      const auto r = accesstoken.find_last_not_of(" \t\r\n");
      if (l != std::string::npos)
        accesstoken = accesstoken.substr(l, r - l + 1);
      else
        accesstoken.clear();
    }
  }
  return accesstoken;
}

// 從 Cookie 取得 Refresh Token
std::string Controller::get_refresh_token(const HTTPRequest& req) {
  std::string refreshtoken;
  auto it = req.headers.find("Cookie");
  if (it != req.headers.end()) {
    const auto& cookieHeader = it->second;
    static const std::string key = "refresh_token=";
    if (auto pos = cookieHeader.find(key); pos != std::string::npos) {
      pos += key.size();
      if (const auto end = cookieHeader.find(';', pos); end == std::string::npos)
        refreshtoken = cookieHeader.substr(pos);
      else
        refreshtoken = cookieHeader.substr(pos, end - pos);
      static constexpr char ws[] = " \t\r\n";
      const auto l = refreshtoken.find_first_not_of(ws);
      const auto r = refreshtoken.find_last_not_of(ws);
      if (l != std::string::npos)
        refreshtoken = refreshtoken.substr(l, r - l + 1);
      else
        refreshtoken.clear();
    }
  }
  if (refreshtoken.size() != 64 ||
      refreshtoken.find_first_not_of("0123456789abcdef") != std::string::npos)
    refreshtoken.clear();

  return refreshtoken;
}
}  // namespace redsafe::apiserver
