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

#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace redsafe::apiserver
{
    namespace beast = boost::beast;
    namespace http = beast::http;
    using json = nlohmann::json;
    using response = http::response<http::string_body>;

    class Controller
    {
    public:
        explicit Controller(http::request<http::string_body> req);

        response handle_request();

    private:
    	/**
		 * @brief 建立 JSON 回應，並可選擇加入 Set‑Cookie
		 *
		 * @param status_code HTTP 狀態碼
		 * @param j           要回傳的 JSON 物件
		 * @return response   Boost.Beast HTTP 回應
		 */
    	static response make_response(int status_code, const json& j);

        /**
		 * @brief 建立 JSON 回應，並可選擇加入 Set‑Cookie
		 *
		 * @param status_code HTTP 狀態碼
		 * @param j           要回傳的 JSON 物件
		 * @param cookie      若非空字串，將寫入 "Set-Cookie" 標頭
		 * @return response   Boost.Beast HTTP 回應
		 */
        static response make_response(int status_code, const json& j, std::string_view cookie);

        http::request<http::string_body> req_;
    };
}