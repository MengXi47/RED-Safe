#pragma once

#include <pqxx/pqxx>

#ifndef SQL_CONNECTION_STR
#define SQL_CONNECTION_STR \
  "host=127.0.0.1 "        \
  "port=5432 "             \
  "dbname=redsafedb "      \
  "user=redsafedb "        \
  "password=redsafedb"
#endif

namespace sql {
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
          "get_ipc_info",
          "SELECT * "
          "FROM connected_ipc "
          "ORDER BY ip_address ASC");
      conn.prepare(
          "create_ipc_info",
          "INSERT INTO connected_ipc "
          "(ip_address, mac_address, ipc_name, custom_name, ipc_account, "
          "ipc_password, fall_sensitivity)"
          "VALUES ($1, $2, $3, $4, $5, $6, $7)");
      conn.prepare(
          "delete_ipc_info", "DELETE FROM connected_ipc WHERE ip_address = $1");
      conn.prepare(
          "update_edge_password",
          "UPDATE config "
          "SET edge_password = $1 "
          "WHERE edge_id = $2");
    } catch (const pqxx::sql_error& e) {
      if (e.sqlstate() != "42P05") {
        throw;
      }
    }
  }
};
} // namespace sql