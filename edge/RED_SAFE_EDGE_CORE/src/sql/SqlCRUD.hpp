#pragma once

#include "SqlConnectionManager.hpp"

#include <folly/dynamic.h>

namespace sql {
class IPCinfo : public ConnectionManager {
 public:
  static std::optional<folly::dynamic> getAll();
  static bool set(
      std::string_view ip_address,
      std::string_view mac_address,
      std::string_view ipc_name,
      std::string_view custom_name,
      std::string_view ipc_account,
      std::string_view ipc_password,
      std::string_view fall_sensitivity);
  static bool del(std::string_view ip_address);
};
class config : public ConnectionManager {
 public:
  static bool updateEdgePassword(
      std::string_view edge_id, std::string_view edge_password);
};
} // namespace sql