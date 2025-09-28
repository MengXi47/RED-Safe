#pragma once

#include <string>
#include <vector>

namespace iptool::domain {

struct NetworkConfig {
  std::string interface_name;
  std::string ip_address;
  std::string mac_address;
  std::string gateway;
  std::string subnet_mask;
  std::vector<std::string> dns_servers;

  [[nodiscard]] bool HasInterface() const noexcept { return !interface_name.empty(); }
  [[nodiscard]] bool HasIp() const noexcept { return !ip_address.empty(); }
  [[nodiscard]] bool HasMac() const noexcept { return !mac_address.empty(); }
  [[nodiscard]] bool HasGateway() const noexcept { return !gateway.empty(); }
  [[nodiscard]] bool HasSubnetMask() const noexcept { return !subnet_mask.empty(); }
  [[nodiscard]] bool HasDns() const noexcept { return !dns_servers.empty(); }
};

}  // namespace iptool::domain
