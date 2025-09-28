#pragma once

#include <optional>
#include <string>

#include "domain/network_config.hpp"

namespace iptool::domain {

class NetworkInfoProvider {
 public:
  virtual ~NetworkInfoProvider() = default;

  virtual NetworkConfig GetConfig(const std::string& interface_name) = 0;
};

}  // namespace iptool::domain
