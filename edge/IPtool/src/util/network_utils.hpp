#pragma once

#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <optional>
#include <string>

struct sockaddr;

namespace iptool::util {

std::string SockaddrToIpv4(const sockaddr* addr);
std::string SockaddrToNetmask(const sockaddr* addr);
std::string SockaddrToMac(const sockaddr* addr);
std::string HexToIpv4(const std::string& hex_value);
std::optional<int> NetmaskToPrefixLength(const std::string& netmask);

}  // namespace iptool::util
