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

#include <utility>

#include "../model/sql/read.hpp"
#include "../model/sql/write.hpp"
#include "../util/IOstream.hpp"
#include "../util/folly_json.hpp"
#include "../util/logger.hpp"
#include "../util/response.hpp"

namespace redsafe::apiserver::service::token {

CreateAccessToken::CreateAccessToken(const std::string_view user_id) : user_id(user_id) {}

int CreateAccessToken::start() {
  try {
    token = jwt::create()
                .set_issuer("RED-Safe")
                .set_subject(util::AESManager::instance().encrypt(user_id))
                .set_issued_at(std::chrono::system_clock::now())
                .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{600})
                .sign(jwt::algorithm::hs256{util::SecretManager::instance().getSecret()});
  } catch (const std::exception& e) {
    util::cerr() << e.what() << std::endl;
    util::log(util::LogFile::server, util::Level::ERROR) << e.what();
    return 1;
  }
  return 0;
}

std::string CreateAccessToken::getAccessToken() const {
  return token;
}

std::string CreateAccessToken::getErrorMessage() const {
  return errorMessage;
}

DecodeAccessToken::DecodeAccessToken(std::string tokenStr) : tokenValue(std::move(tokenStr)) {}

int DecodeAccessToken::start() {
  try {
    // Decode the JWT token string (without immediate verification)
    const auto decoded = jwt::decode(tokenValue);

    // 先檢查是否含有 exp 欄位並檢查過期時間
    if (decoded.has_expires_at()) {
      if (const auto expTime = decoded.get_expires_at();
          std::chrono::system_clock::now() > expTime) {
        errorMessage = "Token 已過期";
        return 1;
      }
    }

    // 驗證 HS256 簽章與 issuer
    jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{util::SecretManager::instance().getSecret()})
        .with_issuer("RED-Safe")
        .verify(decoded);

    // 解析 payload 中的 sub 欄位作為 user_id
    if (decoded.has_payload_claim("sub")) {
      payloadUserId =
          util::AESManager::instance().decrypt(decoded.get_payload_claim("sub").as_string());
      return 0;
    }
    errorMessage = "Token 中缺少 sub 欄位";
    return 2;
  } catch (const std::exception& e) {
    // Capture any decoding or verification errors
    errorMessage = e.what();
    std::cerr << e.what() << std::endl;
    if (errorMessage == "invalid signature") return 3;
    if (errorMessage == "invalid token supplied") return 4;
    return 5;
  }
}

std::string DecodeAccessToken::getUserId() const {
  return payloadUserId;
}

std::string DecodeAccessToken::getErrorMessage() const {
  return errorMessage;
}

CreateRefreshToken::CreateRefreshToken(const std::string_view user_id) : user_id(user_id) {}

int CreateRefreshToken::start() {
  try {
    token = util::generateRandomHex(32);
    WriteToSQL();
  } catch (const std::exception& e) {
    errorMessage = e.what();
    return 1;
  }
  return 0;
}

std::string CreateRefreshToken::getRefreshToken() const {
  return token;
}

std::string CreateRefreshToken::getErrorMessage() const {
  return errorMessage;
}

void CreateRefreshToken::WriteToSQL() const {
  if (model::sql::reg::RefreshTokenRegistrar::start(util::SHA256_HEX(token), user_id) != 0)
    throw std::runtime_error("Failed to reg refresh token in sql");
}

util::Result CheckAndRefreshRefreshToken::run(const std::string& refreshtoken) {
  const std::string user_id =
      model::sql::fin::RefreshTokenRefresher::start(util::SHA256_HEX(refreshtoken));

  if (user_id.empty()) {
    return util::Result{util::status_code::Unauthorized, util::error_code::Refresh_Token_Expired,
                        folly::dynamic::object};
  }

  // 3. 產生新的 Access Token
  CreateAccessToken at(user_id);
  if (at.start() != 0) {
    util::cerr() << at.getErrorMessage();
    util::log(util::LogFile::server, util::Level::ERROR) << at.getErrorMessage();
    return util::Result{util::status_code::InternalServerError,
                        util::error_code::Internal_server_error, folly::dynamic::object};
  }

  return util::Result{util::status_code::Success, util::error_code::Success,
                      make_object({{"access_token", at.getAccessToken()}})};
}

util::Result RevokeRefreshToken::run(const std::string& refreshtoken) {
  if (model::sql::reg::RefreshTokenRevoke::start(util::SHA256_HEX(refreshtoken)) == 1)
    return util::Result{util::status_code::InternalServerError,
                        util::error_code::Internal_server_error, folly::dynamic::object};

  return util::Result{util::status_code::Success, util::error_code::Success,
                      folly::dynamic::object};
}
}  // namespace redsafe::apiserver::service::token

#endif