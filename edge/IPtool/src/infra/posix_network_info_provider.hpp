#pragma once

#include <memory>
#include <string>

#include <ifaddrs.h>

#include "domain/network_info_provider.hpp"
#include "infra/dns_resolver.hpp"

namespace iptool::infra {

class CommandExecutor;

class PosixNetworkInfoProvider final : public domain::NetworkInfoProvider {
 public:
  PosixNetworkInfoProvider(std::shared_ptr<DnsResolver> dns_resolver,
                           std::shared_ptr<CommandExecutor> executor);

  domain::NetworkConfig GetConfig(const std::string& interface_name) override;

 private:
  domain::NetworkConfig BuildFromInterfaces(const std::string& interface_name);
  void PopulateGateway(const std::string& interface_name,
                       domain::NetworkConfig& config);
  void PopulateDns(domain::NetworkConfig& config);

  std::shared_ptr<DnsResolver> dns_resolver_;
  std::shared_ptr<CommandExecutor> executor_;
};

}  // namespace iptool::infra
