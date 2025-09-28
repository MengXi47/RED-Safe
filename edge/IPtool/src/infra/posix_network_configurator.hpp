#pragma once

#include <memory>

#include "domain/network_configurator.hpp"
#include "infra/command_executor.hpp"

namespace iptool::infra {

class CommandExecutor;

class PosixNetworkConfigurator final : public domain::NetworkConfigurator {
 public:
  explicit PosixNetworkConfigurator(std::shared_ptr<CommandExecutor> executor);

  void Apply(const domain::NetworkConfig& config) override;

 private:
  void ApplyLinux(const domain::NetworkConfig& config);
  void ApplyMac(const domain::NetworkConfig& config);
  void ConfigureDns(const domain::NetworkConfig& config);
  void ExecuteAndCheck(const Command& command, const std::string& description);

  std::shared_ptr<CommandExecutor> executor_;
};

}  // namespace iptool::infra
