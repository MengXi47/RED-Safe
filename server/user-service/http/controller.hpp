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

#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace redsafe::server {
namespace beast = boost::beast;
namespace http = beast::http;
using json = nlohmann::json;
using response = http::response<http::string_body>;

class Controller {
 public:
  explicit Controller(http::request<http::string_body> req);

  [[nodiscard]] response handle_request() const;

 private:
  /**
   * @brief 建立 http::request<http::string_body> 回應
   *
   * @param status_code HTTP 狀態碼
   * @param j           要回傳的 JSON 物件
   * @return response   Boost.Beast HTTP 回應
   */
  static response make_response(int status_code, const json& j);

  /**
   * @brief 建立 http::request<http::string_body> 回應
   *
   * @param status_code HTTP 狀態碼
   * @param j           要回傳的 JSON 物件
   * @param cookie      若非空字串，將寫入 "Set-Cookie" 標頭
   * @return response   Boost.Beast HTTP 回應
   */
  static response make_response(
      int status_code, const json& j, std::string_view cookie);

  /**
   * @brief 從 HTTP 請求的 Authorization 標頭中提取並驗證 Access Token
   *
   * 採用 "Bearer <token>" 格式
   *
   * @param req HTTP 請求物件
   * @return 有效的 access token，或空字串
   */
  static std::string get_access_token(
      const http::request<http::string_body>& req);

  /**
   * @brief 從 HTTP 請求的 Cookie 標頭中提取並驗證 Refresh Token
   *
   * 採用從 Cookie 字串中搜尋 "refresh_token="，回傳符合 64
   * 字元小寫十六進位格式的 token， 若格式不符或無法解析，回傳空字串。
   *
   * @param req HTTP 請求物件 (包含可能的 Cookie 標頭)
   * @return 有效的 refresh token，或空字串
   */
  static std::string get_refresh_token(
      const http::request<http::string_body>& req);

  http::request<http::string_body> req_;
};
} // namespace redsafe::apiserver
