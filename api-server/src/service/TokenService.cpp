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

#ifndef REDSAFE_TOKEN_SERVICE_CPP
#define REDSAFE_TOKEN_SERVICE_CPP

#include "TokenService.hpp"
#include "../util/IOstream.hpp"
#include "../util/logger.hpp"

namespace redsafe::apiserver::service::token
{
    CreateAccessToken::CreateAccessToken(std::string_view user_id)
        : user_id(user_id)
    {
    }

    bool CreateAccessToken::start()
    {
        try
        {
            token = jwt::create()
                .set_issuer("RED-Safe")
                .set_subject(user_id)
                .set_issued_at(std::chrono::system_clock::now())
                .set_expires_at(
                    std::chrono::system_clock::now() +
                    std::chrono::seconds{600})
                .sign(jwt::algorithm::hs256{util::loadOrGenerateSecret()});
        }
        catch (const std::exception& e)
        {
            util::cerr() << e.what() << std::endl;
            util::log(util::LogFile::server, util::Level::ERROR) << e.what();
            return false;
        }
        return true;
    }

    std::string CreateAccessToken::getAccessToken() const
    {
        return token;
    }

    std::string CreateAccessToken::getErrorMessage() const
    {
        return errorMessage;
    }

    DecodeAccessToken::DecodeAccessToken(std::string_view tokenStr)
        : tokenValue(tokenStr)
    {
    }

    bool DecodeAccessToken::start()
    {
        try
        {
            // Decode the JWT token string (without immediate verification)
            const auto decoded = jwt::decode(tokenValue);

            // 先檢查是否含有 exp 欄位並檢查過期時間
            if (decoded.has_expires_at()) {
                const auto expTime = decoded.get_expires_at();
                if (std::chrono::system_clock::now() > expTime) {
                    errorMessage = "Token 已過期";
                    return false;
                }
            }

            // 驗證 HS256 簽章與 issuer
            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{util::loadOrGenerateSecret()})
                .with_issuer("RED-Safe")
                .verify(decoded);

            // 解析 payload 中的 sub 欄位作為 user_id
            if (decoded.has_payload_claim("sub")) {
                payloadUserId = decoded.get_payload_claim("sub").as_string();
                return true;
            }
            errorMessage = "Token 中缺少 sub 欄位";
            return false;
        }
        catch (const std::exception& e)
        {
            // Capture any decoding or verification errors
            errorMessage = e.what();
            return false;
        }
    }

    std::string DecodeAccessToken::getUserId() const
    {
        return payloadUserId;
    }

    std::string DecodeAccessToken::getErrorMessage() const
    {
        return errorMessage;
    }
}

#endif