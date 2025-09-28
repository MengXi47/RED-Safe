#include "util/network_utils.hpp"

#include <arpa/inet.h>
#if defined(__linux__)
#include <netpacket/packet.h>
#elif defined(__APPLE__)
#include <net/if_dl.h>
#endif

#include <array>
#include <cstring>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

namespace iptool::util {

namespace {
std::string Ipv4FromInAddr(const in_addr& addr) {
  char buffer[INET_ADDRSTRLEN] = {0};
  if (inet_ntop(AF_INET, &addr, buffer, sizeof(buffer)) == nullptr) {
    return {};
  }
  return buffer;
}
}  // namespace

std::string SockaddrToIpv4(const sockaddr* addr) {
  if (!addr || addr->sa_family != AF_INET) {
    return {};
  }
  auto* ipv4 = reinterpret_cast<const sockaddr_in*>(addr);
  return Ipv4FromInAddr(ipv4->sin_addr);
}

std::string SockaddrToNetmask(const sockaddr* addr) {
  return SockaddrToIpv4(addr);
}

std::string SockaddrToMac(const sockaddr* addr) {
  if (!addr) {
    return {};
  }
#if defined(__linux__)
  if (addr->sa_family == AF_PACKET) {
    const auto* sll = reinterpret_cast<const struct sockaddr_ll*>(addr);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < sll->sll_halen; ++i) {
      if (i != 0) {
        oss << ':';
      }
      oss << std::setw(2)
          << static_cast<unsigned>(static_cast<unsigned char>(sll->sll_addr[i]));
    }
    return oss.str();
  }
#elif defined(__APPLE__)
  if (addr->sa_family == AF_LINK) {
    const auto* sdl = reinterpret_cast<const struct sockaddr_dl*>(addr);
    const unsigned char* mac = reinterpret_cast<const unsigned char*>(
        LLADDR(sdl));
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < sdl->sdl_alen; ++i) {
      if (i != 0) {
        oss << ':';
      }
      oss << std::setw(2) << static_cast<unsigned>(mac[i]);
    }
    return oss.str();
  }
#endif
  return {};
}

std::string HexToIpv4(const std::string& hex_value) {
  if (hex_value.size() != 8) {
    return {};
  }
  std::array<unsigned char, 4> bytes{};
  for (size_t i = 0; i < bytes.size(); ++i) {
    const auto byte_str = hex_value.substr(i * 2, 2);
    try {
      const auto byte_value =
          static_cast<unsigned char>(std::stoul(byte_str, nullptr, 16));
      bytes[i] = byte_value;
    } catch (const std::exception&) {
      return {};
    }
  }

  unsigned char packed[4] = {bytes[3], bytes[2], bytes[1], bytes[0]};
  in_addr addr{};
  std::memcpy(&addr.s_addr, packed, sizeof(packed));
  return Ipv4FromInAddr(addr);
}

std::optional<int> NetmaskToPrefixLength(const std::string& netmask) {
  if (netmask.empty()) {
    return std::nullopt;
  }

  in_addr addr{};
  if (inet_pton(AF_INET, netmask.c_str(), &addr) != 1) {
    return std::nullopt;
  }

  const uint32_t value = ntohl(addr.s_addr);
  bool seen_zero = false;
  int prefix = 0;
  for (int i = 31; i >= 0; --i) {
    if (value & (1u << i)) {
      if (seen_zero) {
        return std::nullopt;
      }
      ++prefix;
    } else {
      seen_zero = true;
    }
  }
  return prefix;
}

}  // namespace iptool::util
