#include "infra/posix_network_info_provider.hpp"

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "common/logging.hpp"
#include "domain/network_error.hpp"
#include "infra/command_executor.hpp"
#include "util/network_utils.hpp"

namespace iptool::infra {

namespace {
std::string Trim(std::string value) {
  const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
  value.erase(value.begin(),
              std::find_if(value.begin(), value.end(), not_space));
  value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(),
              value.end());
  return value;
}
}

PosixNetworkInfoProvider::PosixNetworkInfoProvider(
    std::shared_ptr<DnsResolver> dns_resolver,
    std::shared_ptr<CommandExecutor> executor)
    : dns_resolver_(std::move(dns_resolver)),
      executor_(std::move(executor)) {}

domain::NetworkConfig PosixNetworkInfoProvider::GetConfig(
    const std::string& interface_name) {
  auto config = BuildFromInterfaces(interface_name);
  PopulateGateway(config.interface_name, config);
  PopulateDns(config);
  return config;
}

domain::NetworkConfig PosixNetworkInfoProvider::BuildFromInterfaces(
    const std::string& interface_name) {
  struct ifaddrs* ifaddr = nullptr;
  if (getifaddrs(&ifaddr) == -1) {
    throw domain::NetworkError("getifaddrs failed");
  }

  std::unique_ptr<struct ifaddrs, decltype(&freeifaddrs)> guard(ifaddr,
                                                               freeifaddrs);

  domain::NetworkConfig config;
  config.interface_name = interface_name;
  bool found = false;

  for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa || !ifa->ifa_name) {
      continue;
    }
    if (interface_name != ifa->ifa_name) {
      continue;
    }
    found = true;

    if (ifa->ifa_addr) {
      if (ifa->ifa_addr->sa_family == AF_INET) {
        const auto ip = util::SockaddrToIpv4(ifa->ifa_addr);
        if (!ip.empty()) {
          config.ip_address = ip;
        }
        const auto mask = util::SockaddrToNetmask(ifa->ifa_netmask);
        if (!mask.empty()) {
          config.subnet_mask = mask;
        }
      }

      const auto mac = util::SockaddrToMac(ifa->ifa_addr);
      if (!mac.empty()) {
        config.mac_address = mac;
      }
    }
  }

  if (!found) {
    throw domain::NetworkInterfaceNotFoundError(interface_name);
  }

  return config;
}

void PosixNetworkInfoProvider::PopulateGateway(const std::string& interface_name,
                                               domain::NetworkConfig& config) {
#if defined(__linux__)
  std::ifstream route_file("/proc/net/route");
  if (!route_file.is_open()) {
    LogWarn("Unable to open /proc/net/route for gateway lookup");
    return;
  }

  std::string line;
  std::getline(route_file, line);  // header

  while (std::getline(route_file, line)) {
    std::istringstream iss(line);
    std::string iface;
    std::string destination;
    std::string gateway;
    std::string flags_str;
    if (!(iss >> iface >> destination >> gateway >> flags_str)) {
      continue;
    }
    if (iface != interface_name || destination != "00000000") {
      continue;
    }
    const auto gateway_ip = util::HexToIpv4(gateway);
    if (!gateway_ip.empty()) {
      config.gateway = gateway_ip;
      return;
    }
  }
#elif defined(__APPLE__)
  if (!executor_) {
    return;
  }
  const Command command{.executable = "/sbin/route",
                        .arguments = {"-n", "get", "default"}};
  const auto result = executor_->Execute(command);
  if (result.exit_code != 0) {
    LogWarnFormat("route returned non-zero exit code {}", result.exit_code);
    return;
  }

  std::istringstream stream(result.output);
  std::string line;
  while (std::getline(stream, line)) {
    const auto pos = line.find("gateway:");
    if (pos == std::string::npos) {
      continue;
    }
    auto value = Trim(line.substr(pos + 8));
    if (!value.empty()) {
      config.gateway = value;
      return;
    }
  }
#else
  (void)interface_name;
  (void)config;
#endif
}

void PosixNetworkInfoProvider::PopulateDns(domain::NetworkConfig& config) {
  if (!dns_resolver_) {
    return;
  }

  config.dns_servers = dns_resolver_->GetSystemDnsServers();
}

}  // namespace iptool::infra
