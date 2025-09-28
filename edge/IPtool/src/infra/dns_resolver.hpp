#pragma once

#include <string>
#include <vector>

namespace iptool::infra {

class DnsResolver {
 public:
  virtual ~DnsResolver() = default;

  virtual std::vector<std::string> GetSystemDnsServers() = 0;
};

class ResolvConfDnsResolver final : public DnsResolver {
 public:
  std::vector<std::string> GetSystemDnsServers() override;
};

}  // namespace iptool::infra
