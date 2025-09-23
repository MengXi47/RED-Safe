#pragma once

#include <string>
#include <utility>
#include <vector>

namespace ipcscan {

class IDiscoveryTransport {
 public:
  virtual ~IDiscoveryTransport() = default;
  // 執行Probe並回傳來源IP與原始回應
  virtual std::vector<std::pair<std::string, std::string>> Probe() = 0;
};

}  // namespace ipcscan
