#pragma once

#include "domain/network_config.hpp"

namespace iptool::domain {

class NetworkConfigurator {
 public:
  virtual ~NetworkConfigurator() = default;

  virtual void Apply(const NetworkConfig& config) = 0;
};

}  // namespace iptool::domain
