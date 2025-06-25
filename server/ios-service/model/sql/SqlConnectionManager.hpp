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

#ifndef REDSAFE_SQLCONNECTIONMANAGER_HPP
#define REDSAFE_SQLCONNECTIONMANAGER_HPP

#include <memory>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>

#include "../../config.hpp"

namespace redsafe::server::model::sql {
class ConnectionManager {
 public:
  static void initConnection(const std::string& conn_str) {
    if (!conn_) {
      conn_ = std::make_unique<pqxx::connection>(conn_str);
      if (!conn_->is_open()) {
        throw std::runtime_error("DB connection failed: " + conn_str);
      }
      SQLinit(*conn_);
    }
  }

  static pqxx::connection& connection() {
    if (!conn_) {
      initConnection(SQL_CONNECTION_STR);
    }
    return *conn_;
  }

 private:
  static inline thread_local std::unique_ptr<pqxx::connection> conn_{nullptr};

  static void SQLinit(pqxx::connection& conn) {
    try {
      conn.prepare(
          "register_edge",
          "INSERT INTO edge_devices "
          "(edge_serial_number, version) "
          "VALUES ($1, $2) "
          "ON CONFLICT (edge_serial_number) DO NOTHING");
      conn.prepare(
          "register_user",
          "INSERT INTO users "
          "(email, user_name, user_password_hash) "
          "VALUES ($1, $2, $3) "
          "ON CONFLICT DO NOTHING");
      conn.prepare(
          "find_user_id", "SELECT user_id FROM users WHERE email = $1");
      conn.prepare(
          "find_user_name_email",
          "SELECT user_name FROM users WHERE email = $1");
      conn.prepare(
          "find_user_name_userid",
          "SELECT user_name FROM users WHERE user_id = $1");
      conn.prepare("find_email", "SELECT email FROM users WHERE user_id = $1");
      conn.prepare(
          "register_ios_device",
          "INSERT INTO ios_devices "
          "(ios_device_id, user_id, apns_token, device_name, last_seen_at) "
          "VALUES (COALESCE(NULLIF($1,'')::uuid, gen_random_uuid()), $2, $3, "
          "$4, NOW()) "
          "ON CONFLICT (ios_device_id) DO UPDATE "
          "SET user_id      = EXCLUDED.user_id, "
          "    apns_token   = EXCLUDED.apns_token, "
          "    device_name  = EXCLUDED.device_name, "
          "    last_seen_at = NOW()");
      conn.prepare(
          "find_ios_device_id",
          "SELECT ios_device_id FROM ios_devices WHERE apns_token = $1");
      conn.prepare(
          "bind_edge_user",
          "INSERT INTO edge_users "
          "(edge_serial_number, user_id) "
          "VALUES ($1, $2) "
          "ON CONFLICT DO NOTHING");
      conn.prepare(
          "unbind_edge_user",
          "DELETE FROM edge_users "
          "WHERE edge_serial_number = $1 "
          "AND   user_id      = $2");
      conn.prepare(
          "find_user_pwdhash",
          "SELECT user_password_hash "
          "FROM users "
          "WHERE email = $1");
      conn.prepare(
          "find_user_edges",
          "SELECT edge_serial_number "
          "FROM edge_users "
          "WHERE user_id = $1");
      conn.prepare(
          "reg_refretoken",
          "INSERT INTO auth "
          "(refresh_token_hash, user_id, expires_at) "
          "VALUES ($1, $2, NOW() + INTERVAL '30 days')");
      conn.prepare(
          "chk_refretoken",
          "WITH upd AS ( "
          "    UPDATE auth "
          "    SET    expires_at = NOW() + INTERVAL '30 days' "
          "    WHERE  refresh_token_hash = $1 "
          "      AND  revoked = FALSE "
          "      AND  expires_at > NOW() "
          "    RETURNING user_id "
          "), rev AS ( "
          "    UPDATE auth "
          "    SET    revoked = TRUE "
          "    WHERE  refresh_token_hash = $1 "
          "      AND  revoked = FALSE "
          "      AND  expires_at <= NOW() "
          ") "
          "SELECT user_id FROM upd");
      conn.prepare(
          "revoke_refretoken",
          "UPDATE auth "
          "SET revoked = TRUE "
          "WHERE refresh_token_hash = $1");

    } catch (const pqxx::sql_error& e) {
      if (e.sqlstate() != "42P05") {
        throw;
      }
    }
  }
};
} // namespace redsafe::apiserver::model::sql

#endif