#include "infra/posix_network_configurator.hpp"

#include <fstream>
#include <sstream>
#include <string>

#include "common/logging.hpp"
#include "domain/network_error.hpp"
#include "infra/command_executor.hpp"
#include "util/network_utils.hpp"

namespace iptool::infra {

PosixNetworkConfigurator::PosixNetworkConfigurator(
    std::shared_ptr<CommandExecutor> executor)
    : executor_(std::move(executor)) {}

void PosixNetworkConfigurator::Apply(const domain::NetworkConfig& config) {
  if (!executor_) {
    throw domain::NetworkError("No command executor provided");
  }
  if (!config.HasInterface()) {
    throw domain::NetworkError("Interface is required for configuration");
  }

  LogInfoFormat("Applying configuration to interface '{}'", config.interface_name);

#if defined(__linux__)
  ApplyLinux(config);
#elif defined(__APPLE__)
  ApplyMac(config);
#else
  throw domain::UnsupportedPlatformError();
#endif

  ConfigureDns(config);
}

void PosixNetworkConfigurator::ApplyLinux(
    const domain::NetworkConfig& config) {
  const auto& iface = config.interface_name;

  if (config.HasMac()) {
    ExecuteAndCheck(Command{"ip",
                             {"link", "set", "dev", iface, "address",
                              config.mac_address}},
                    "set mac address");
  }

  if (config.HasIp()) {
    std::string ip_assignment = config.ip_address;
    if (config.HasSubnetMask()) {
      const auto prefix = util::NetmaskToPrefixLength(config.subnet_mask);
      if (prefix.has_value()) {
        ip_assignment += "/" + std::to_string(*prefix);
      } else {
        LogWarnFormat("Invalid subnet mask '{}'", config.subnet_mask);
      }
    }

    ExecuteAndCheck(Command{"ip",
                             {"addr", "replace", ip_assignment, "dev", iface}},
                    "configure ip address");
    ExecuteAndCheck(Command{"ip", {"link", "set", "dev", iface, "up"}},
                    "bring interface up");
  }

  if (config.HasGateway()) {
    ExecuteAndCheck(Command{"ip",
                             {"route", "replace", "default", "via",
                              config.gateway, "dev", iface}},
                    "configure gateway");
  }
}

void PosixNetworkConfigurator::ApplyMac(
    const domain::NetworkConfig& config) {
  const auto& iface = config.interface_name;

  if (config.HasMac()) {
    ExecuteAndCheck(Command{"/sbin/ifconfig",
                             {iface, "ether", config.mac_address}},
                    "set mac address");
  }

  if (config.HasIp() && config.HasSubnetMask()) {
    ExecuteAndCheck(Command{"/sbin/ifconfig",
                             {iface, "inet", config.ip_address, "netmask",
                              config.subnet_mask}},
                    "configure ip address");
  } else if (config.HasIp()) {
    ExecuteAndCheck(Command{"/sbin/ifconfig",
                             {iface, "inet", config.ip_address}},
                    "configure ip address");
  }

  if (config.HasGateway()) {
    const auto change_result = executor_->Execute(Command{
        .executable = "/sbin/route",
        .arguments = {"-n", "change", "default", config.gateway}});
    if (change_result.exit_code != 0) {
      LogWarnFormat("route change default failed: {}", change_result.output);
      ExecuteAndCheck(Command{"/sbin/route", {"-n", "add", "default",
                                               config.gateway}},
                      "add default route");
    }
  }
}

void PosixNetworkConfigurator::ConfigureDns(
    const domain::NetworkConfig& config) {
  if (!config.HasDns()) {
    return;
  }

  std::ofstream resolv("/etc/resolv.conf", std::ios::trunc);
  if (!resolv.is_open()) {
    throw domain::NetworkError("Unable to open /etc/resolv.conf for writing");
  }

  for (const auto& dns : config.dns_servers) {
    resolv << "nameserver " << dns << '\n';
  }

  LogInfo("Updated DNS configuration in /etc/resolv.conf");
}

void PosixNetworkConfigurator::ExecuteAndCheck(const Command& command,
                                               const std::string& description) {
  const auto result = executor_->Execute(command);
  if (result.exit_code != 0) {
    throw domain::NetworkCommandError(command.executable + " " + description,
                                      result.exit_code, result.output);
  }
}

}  // namespace iptool::infra
