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

#include "TokenService.hpp"

#include <utility>
#include <nlohmann/json.hpp>

#include <jwt.h>
#include "../grpc/AuthClient.hpp"
#include "../model/sql/read.hpp"
#include "../model/sql/write.hpp"
#include "../util/IOstream.hpp"
#include "../util/logger.hpp"
#include "../util/response.hpp"

namespace redsafe::server::service::token {
using json = nlohmann::json;

CreateAccessToken::CreateAccessToken(const std::string_view user_id)
  : user_id(user_id) {
}

int CreateAccessToken::start() {
  try {
    token =
        jwt::create()
        .set_issuer("RED-Safe")
        .set_subject(util::AESManager::instance().encrypt(user_id))
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(
            std::chrono::system_clock::now() + std::chrono::seconds{600})
        .sign(
            jwt::algorithm::hs256{
                util::SecretManager::instance().getSecret()});
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

DecodeAccessToken::DecodeAccessToken(std::string tokenStr)
  : tokenValue(std::move(tokenStr)) {
}

int DecodeAccessToken::start() {
  server::grpc::AuthClient client("localhost:50051");
  return client.Decode(tokenValue, payloadUserId, errorMessage);
}

std::string DecodeAccessToken::getUserId() const {
  return payloadUserId;
}

std::string DecodeAccessToken::getErrorMessage() const {
  return errorMessage;
}

CreateRefreshToken::CreateRefreshToken(const std::string_view user_id)
  : user_id(user_id) {
}

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
  if (model::sql::reg::RefreshTokenRegistrar::start(
      util::SHA256_HEX(token),
      user_id) != 0) {
    throw std::runtime_error("Failed to reg refresh token in sql");
  }
}

util::Result CheckAndRefreshRefreshToken::run(const std::string& refreshtoken) {
  const std::string user_id = model::sql::fin::RefreshTokenRefresher::start(
      util::SHA256_HEX(refreshtoken));

  if (user_id.empty()) {
    return util::Result{
        util::status_code::Unauthorized,
        util::error_code::Refresh_Token_Expired,
        json{}};
  }

  // 3. 產生新的 Access Token
  CreateAccessToken at(user_id);
  if (at.start() != 0) {
    util::cerr() << at.getErrorMessage();
    util::log(util::LogFile::server, util::Level::ERROR)
        << at.getErrorMessage();
    return util::Result{
        util::status_code::InternalServerError,
        util::error_code::Internal_server_error,
        json{}};
  }

  return util::Result{
      util::status_code::Success,
      util::error_code::Success,
      json{{"access_token", at.getAccessToken()}}};
}

util::Result RevokeRefreshToken::run(const std::string& refreshtoken) {
  if (model::sql::reg::RefreshTokenRevoke::start(
      util::SHA256_HEX(refreshtoken)) == 1) {
    return util::Result{
        util::status_code::InternalServerError,
        util::error_code::Internal_server_error,
        json{}};
  }

  return util::Result{
      util::status_code::Success, util::error_code::Success, json{}};
}
} // namespace redsafe::apiserver::service::token
