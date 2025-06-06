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

#include <folly/experimental/coro/Task.h>

#include "../util/folly_json.hpp"
#include "../util/http_types.hpp"

namespace redsafe::apiserver {
/**
 * @brief 依據請求內容呼叫對應服務的控制器
 */
class Controller {
 public:
  /**
   * @brief 以請求資料建構控制器
   * @param req HTTP 請求資料
   */
  explicit Controller(HTTPRequest req);

  /**
   * @brief 處理請求並產生回應
   * @return 非同步回應任務
   */
  [[nodiscard]] folly::coro::Task<HTTPResponse> handle_request() const;

 private:
  /**
   * @brief 組合回應物件
   * @param status_code HTTP 狀態碼
   * @param j 回應 JSON
   */
  static HTTPResponse make_response(int status_code, const json& j);

  /**
   * @brief 組合回應並夾帶 Cookie
   * @param status_code HTTP 狀態碼
   * @param j 回應 JSON
   * @param cookie Set-Cookie 字串
   */
  static HTTPResponse make_response(int status_code, const json& j, std::string_view cookie);

  /**
   * @brief 從標頭取得 Access Token
   * @param req HTTP 請求
   * @return token 字串
   */
  [[nodiscard]] static std::string get_access_token(const HTTPRequest& req);

  /**
   * @brief 從 Cookie 取得 Refresh Token
   * @param req HTTP 請求
   * @return token 字串
   */
  [[nodiscard]] static std::string get_refresh_token(const HTTPRequest& req);

  HTTPRequest req_;
};
}  // namespace redsafe::apiserver
