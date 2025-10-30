#include "SqlCRUD.hpp"

#include <util/logging.hpp>

namespace sql {

std::optional<folly::dynamic> IPCinfo::getAll() {
  try {
    pqxx::work tx{connection()};
    const pqxx::result r = tx.exec(pqxx::prepped("get_ipc_info"));
    if (r.empty()) {
      return std::nullopt;
    }
    folly::dynamic ipc_list = folly::dynamic::array();
    for (const auto& row : r) {
      folly::dynamic ipc = folly::dynamic::object;
      ipc["ip_address"] = row["ip_address"].c_str();
      ipc["mac_address"] = row["mac_address"].c_str();
      ipc["ipc_name"] =
          row["ipc_name"].is_null() ? "" : row["ipc_name"].c_str();
      ipc["custom_name"] =
          row["custom_name"].is_null() ? "" : row["custom_name"].c_str();
      ipc["ipc_account"] =
          row["ipc_account"].is_null() ? "" : row["ipc_account"].c_str();
      ipc["ipc_password"] =
          row["ipc_password"].is_null() ? "" : row["ipc_password"].c_str();
      ipc["fall_sensitivity"] = row["fall_sensitivity"].is_null()
          ? 0
          : row["fall_sensitivity"].as<int>();
      ipc_list.push_back(std::move(ipc));
    }
    return ipc_list;
  } catch (const pqxx::sql_error& e) {
    LogErrorFormat(
        "PostgreSQL error in getAll(): {} (SQLSTATE: {} )",
        e.what(),
        e.sqlstate());
    return std::nullopt;
  } catch (const std::exception& e) {
    LogErrorFormat("PostgreSQL error in getAll(): {}", e.what());
    return std::nullopt;
  }
}
bool IPCinfo::set(
    std::string_view ip_address,
    std::string_view mac_address,
    std::string_view ipc_name,
    std::string_view custom_name,
    std::string_view ipc_account,
    std::string_view ipc_password,
    std::string_view fall_sensitivity) {
  try {
    pqxx::work tx{connection()};
    const auto r = tx.exec(
        pqxx::prepped("create_ipc_info"),
        pqxx::params(
            ip_address,
            mac_address,
            ipc_name,
            custom_name,
            ipc_account,
            ipc_password,
            fall_sensitivity));
    if (r.affected_rows() == 0) {
      return false;
    }
    tx.commit();
    return true;
  } catch (const pqxx::sql_error& e) {
    LogErrorFormat("PostgreSQL error in set(): {}", e.what());
    return false;
  }
}

bool IPCinfo::del(std::string_view ip_address) {
  try {
    pqxx::work tx{connection()};
    const auto r =
        tx.exec(pqxx::prepped("delete_ipc_info"), pqxx::params(ip_address));
    if (r.affected_rows() == 0) {
      return false;
    }
    tx.commit();
  } catch (const pqxx::sql_error& e) {
    LogErrorFormat("PostgreSQL error in del(): {}", e.what());
    return false;
  }
  return true;
}

bool config::updateEdgePassword(
    std::string_view edge_id, std::string_view edge_password) {
  try {
    pqxx::work tx{connection()};
    const auto r = tx.exec(
        pqxx::prepped("update_edge_password"),
        pqxx::params(edge_password, edge_id));
    if (r.affected_rows() == 0) {
      return false;
    }
    tx.commit();
  } catch (pqxx::sql_error& e) {
    LogErrorFormat("PostgreSQL error in updateEdgePassword(): {}", e.what());
  }
  return true;
}
} // namespace sql