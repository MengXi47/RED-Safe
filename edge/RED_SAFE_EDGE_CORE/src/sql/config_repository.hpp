#pragma once

#include <optional>
#include <string>

namespace sql {

// 從 PostgreSQL 讀取唯一一筆 Edge 設定，回傳 edge_id。
std::optional<std::string> LoadEdgeId();

} // namespace sql
