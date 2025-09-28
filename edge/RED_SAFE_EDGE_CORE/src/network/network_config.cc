#include "network/network_config.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <ifaddrs.h>
#include <iomanip>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

#if defined(__linux__)
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <net/if_dl.h>
#endif

namespace network {
namespace {

std::string FormatMac(const unsigned char* data, std::size_t length) {
  std::ostringstream oss;
  for (std::size_t i = 0; i < length; ++i) {
    if (i != 0) {
      oss << ':';
    }
    oss << std::hex << std::nouppercase << std::setw(2) << std::setfill('0')
        << static_cast<int>(data[i]);
  }
  return oss.str();
}

std::vector<std::string> ParseDnsServers() {
  std::vector<std::string> servers;
  std::ifstream file("/etc/resolv.conf");
  if (!file.is_open()) {
    return servers;
  }
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string keyword;
    iss >> keyword;
    if (keyword == "nameserver") {
      std::string addr;
      iss >> addr;
      if (!addr.empty()) {
        servers.push_back(addr);
      }
    }
  }
  return servers;
}

std::string ReadGatewayLinux(const std::string& iface) {
#if defined(__linux__)
  std::ifstream file("/proc/net/route");
  if (!file.is_open()) {
    return {};
  }
  std::string line;
  std::getline(file, line);  // header
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string ifname, destination, gateway;
    unsigned flags = 0;
    if (!(iss >> ifname >> destination >> gateway >> flags)) {
      continue;
    }
    if (ifname == iface && destination == "00000000" && (flags & 0x2)) {
      unsigned long gw_hex = 0;
      std::istringstream hex_stream(gateway);
      hex_stream >> std::hex >> gw_hex;
      std::ostringstream addr;
      addr << ((gw_hex) & 0xFF) << '.' << ((gw_hex >> 8) & 0xFF) << '.'
           << ((gw_hex >> 16) & 0xFF) << '.' << ((gw_hex >> 24) & 0xFF);
      return addr.str();
    }
  }
#endif
  return {};
}

std::string ReadGatewayMac() {
#if defined(__APPLE__)
  std::string output;
  FILE* pipe = popen("/usr/sbin/netstat -nr | awk '/default/ {print $2; exit}'", "r");
  if (!pipe) {
    return {};
  }
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    output += buffer;
  }
  pclose(pipe);
  if (!output.empty() && output.back() == '\n') {
    output.pop_back();
  }
  return output;
#else
  return {};
#endif
}

bool UpdateResolvers(const std::vector<std::string>& dns_servers,
                     std::string& error) {
  if (dns_servers.empty()) {
    return true;
  }
  std::ofstream file("/etc/resolv.conf", std::ios::trunc);
  if (!file.is_open()) {
    error = "Unable to write /etc/resolv.conf";
    return false;
  }
  for (const auto& dns : dns_servers) {
    file << "nameserver " << dns << '\n';
  }
  return true;
}

int NetmaskToPrefix(const std::string& netmask) {
  struct in_addr addr{};
  if (inet_aton(netmask.c_str(), &addr) == 0) {
    return -1;
  }
  uint32_t mask = ntohl(addr.s_addr);
  int count = 0;
  bool zero_found = false;
  for (int i = 31; i >= 0; --i) {
    bool bit = (mask >> i) & 1U;
    if (bit) {
      if (zero_found) {
        return -1;
      }
      ++count;
    } else {
      zero_found = true;
    }
  }
  return count;
}

bool RunCommand(const std::string& command,
                std::string& error,
                bool ignore_failure = false) {
  int result = std::system(command.c_str());
  if (result != 0 && !ignore_failure) {
    std::ostringstream oss;
    oss << "Command failed: " << command << " (exit code " << result << ")";
    error = oss.str();
    return false;
  }
  return true;
}

}  // namespace

bool FetchNetworkConfig(const std::string& interface,
                        NetworkConfig& config,
                        std::string& error) {
  struct ifaddrs* ifaddr = nullptr;
  if (getifaddrs(&ifaddr) == -1) {
    error = std::strerror(errno);
    return false;
  }

  std::string target_interface;
  for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }
    bool is_loopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;
    if (!interface.empty()) {
      if (interface == ifa->ifa_name) {
        target_interface = ifa->ifa_name;
        break;
      }
    } else if (!is_loopback) {
      target_interface = ifa->ifa_name;
      break;
    }
  }

  if (target_interface.empty()) {
    error = interface.empty() ? "No active network interface found"
                              : ("Interface not found: " + interface);
    freeifaddrs(ifaddr);
    return false;
  }

  config.interface = target_interface;

#if defined(__linux__)
  // Acquire MAC address via ioctl.
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock >= 0) {
    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, target_interface.c_str(), IFNAMSIZ - 1);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
      config.mac = FormatMac(
          reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data), 6);
    }
    close(sock);
  }
#endif

  for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr || target_interface != ifa->ifa_name) {
      continue;
    }
    int family = ifa->ifa_addr->sa_family;
    if (family == AF_INET) {
      char addr_buf[INET_ADDRSTRLEN];
      if (inet_ntop(family,
                    &reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr)->sin_addr,
                    addr_buf,
                    sizeof(addr_buf))) {
        config.ip = addr_buf;
      }
      if (ifa->ifa_netmask) {
        if (inet_ntop(family,
                      &reinterpret_cast<struct sockaddr_in*>(ifa->ifa_netmask)->sin_addr,
                      addr_buf,
                      sizeof(addr_buf))) {
          config.netmask = addr_buf;
        }
      }
    }
#if defined(__APPLE__)
    else if (family == AF_LINK) {
      const auto* sdl = reinterpret_cast<struct sockaddr_dl*>(ifa->ifa_addr);
      if (sdl && sdl->sdl_alen > 0) {
        config.mac = FormatMac(
            reinterpret_cast<const unsigned char*>(LLADDR(sdl)), sdl->sdl_alen);
      }
    }
#endif
  }

  freeifaddrs(ifaddr);

#if defined(__APPLE__)
  config.gateway = ReadGatewayMac();
#else
  config.gateway = ReadGatewayLinux(target_interface);
#endif
  config.dns_servers = ParseDnsServers();
  return true;
}

bool ApplyNetworkConfig(const NetworkConfig& config, std::string& error) {
#if defined(__linux__)
  int prefix = NetmaskToPrefix(config.netmask);
  if (prefix < 0) {
    error = "Invalid netmask";
    return false;
  }
  if (!RunCommand("ip addr flush dev " + config.interface, error)) {
    return false;
  }
  if (!RunCommand("ip addr add " + config.ip + "/" + std::to_string(prefix) +
                      " dev " + config.interface,
                  error)) {
    return false;
  }
  if (!config.mac.empty()) {
    if (!RunCommand("ip link set dev " + config.interface + " address " + config.mac,
                    error)) {
      return false;
    }
  }
  if (!config.gateway.empty()) {
    if (!RunCommand("ip route replace default via " + config.gateway +
                        " dev " + config.interface,
                    error)) {
      return false;
    }
  }
  if (!config.dns_servers.empty()) {
    if (!UpdateResolvers(config.dns_servers, error)) {
      return false;
    }
  }
  return true;
#elif defined(__APPLE__)
  if (!RunCommand("ifconfig " + config.interface + " inet " + config.ip +
                      " netmask " + config.netmask,
                  error)) {
    return false;
  }
  if (!config.mac.empty()) {
    if (!RunCommand("ifconfig " + config.interface + " ether " + config.mac,
                    error)) {
      return false;
    }
  }
  if (!config.gateway.empty()) {
    RunCommand("route -n delete default 2>/dev/null", error, true);
    if (!RunCommand("route -n add default " + config.gateway, error)) {
      return false;
    }
  }
  if (!config.dns_servers.empty()) {
    if (!UpdateResolvers(config.dns_servers, error)) {
      return false;
    }
  }
  return true;
#else
  error = "Network configuration not supported on this platform";
  return false;
#endif
}

}  // namespace network
