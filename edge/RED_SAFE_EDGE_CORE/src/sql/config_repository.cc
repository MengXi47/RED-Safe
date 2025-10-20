#include "sql/config_repository.hpp"

#include "util/logging.hpp"

#include <pqxx/pqxx>

#include <optional>
#include <string>

namespace {
constexpr const char* kConnectionUri =
    "postgresql://redsafedb:redsafedb@localhost/redsafedb";
constexpr const char* kSelectEdgeId = "SELECT edge_id FROM config LIMIT 1";
} // namespace

namespace sql {

std::optional<std::string> LoadEdgeId() {
  try {
    pqxx::connection connection{kConnectionUri};
    if (!connection.is_open()) {
      LogError("無法連線至 PostgreSQL，連線已關閉");
      return std::nullopt;
    }

    pqxx::read_transaction txn{connection};
    pqxx::result result = txn.exec(kSelectEdgeId);
    txn.commit();

    if (result.empty()) {
      LogWarn("config 資料表沒有任何資料");
      return std::nullopt;
    }

    const auto& row = result.front();
    const auto field = row["edge_id"];
    if (field.is_null()) {
      LogWarn("config.edge_id 為 NULL");
      return std::nullopt;
    }

    std::string edge_id = field.c_str();
    if (edge_id.empty()) {
      LogWarn("config.edge_id 為空字串");
      return std::nullopt;
    }

    return edge_id;
  } catch (const pqxx::sql_error& ex) {
    LogErrorFormat("查詢 edge_id 失敗: {}", ex.what());
  } catch (const std::exception& ex) {
    LogErrorFormat("連線 PostgreSQL 失敗: {}", ex.what());
  } catch (...) {
    LogError("連線 PostgreSQL 發生未知錯誤");
  }
  return std::nullopt;
}

} // namespace sql
