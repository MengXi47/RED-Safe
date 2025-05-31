/******************************************************************************
Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
******************************************************************************/

#ifndef REDSAFE_TOKEN_SERVICE_HPP
#define REDSAFE_TOKEN_SERVICE_HPP

#include "../util/crypto.hpp"
#include "../../lib/jwt-cpp/jwt.h"

namespace redsafe::apiserver::service::token
{
    class CreateAccessToken
    {
    public:
        /**
         * \brief 創建 Access_Token 使用 JWT
         *
         *  run start
         *
         * \param user_id 使用者 user_id
         * \return std::string token
         */
        explicit CreateAccessToken(std::string_view user_id);
        // 1 -> 成功, 0 -> 失敗
        [[nodiscard]] bool start();
        [[nodiscard]] std::string getAccessToken() const;
        [[nodiscard]] std::string getErrorMessage() const;
    private:
        std::string user_id;
        std::string token;
        std::string errorMessage;
    };

    /**
     * \brief 解碼並驗證 Access Token（JWT）
     *
     * 用於解析傳入的 JWT，並驗證簽名與有效期
     */
    class DecodeAccessToken
    {
    public:
        /**
         * \brief 建構子，接收要解碼的 JWT 字串
         * \param tokenStr 要解碼的 Access Token
         */
        explicit DecodeAccessToken(std::string_view tokenStr);

        /**
         * \brief 執行解碼與驗證
         * \return true 若驗證通過，並將 user_id 存到 payloadUserId；false 表示驗證失敗
         */
        [[nodiscard]] bool start();

        /**
         * \brief 取得解碼後的 user_id (sub claim)
         * \return std::string 解碼後的 user_id，若未通過驗證則為空字串
         */
        [[nodiscard]] std::string getUserId() const;

        /**
         * \brief 取得驗證失敗時的錯誤訊息
         * \return std::string 解驗證失敗時的錯誤描述
         */
        [[nodiscard]] std::string getErrorMessage() const;

    private:
        std::string tokenValue;
        std::string payloadUserId;
        std::string errorMessage;
    };


}

#endif