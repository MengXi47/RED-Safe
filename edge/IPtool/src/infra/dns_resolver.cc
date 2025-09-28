#include "infra/dns_resolver.hpp"

#include <fstream>
#include <sstream>

#include "common/logging.hpp"

namespace iptool::infra {

std::vector<std::string> ResolvConfDnsResolver::GetSystemDnsServers() {
  std::ifstream file("/etc/resolv.conf");
  if (!file.is_open()) {
    LogWarn("Unable to open /etc/resolv.conf");
    return {};
  }

  std::vector<std::string> servers;
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }
    std::istringstream iss(line);
    std::string key;
    std::string value;
    if (iss >> key >> value) {
      if (key == "nameserver") {
        servers.push_back(value);
      }
    }
  }

  return servers;
}

}  // namespace iptool::infra
