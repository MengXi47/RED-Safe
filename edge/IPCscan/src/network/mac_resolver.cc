#include "network/mac_resolver.hpp"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

#if defined(__APPLE__)
#include <net/if_dl.h>
#include <net/route.h>
#include <sys/sysctl.h>
#else
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace ipcscan {

std::optional<std::string> MacResolver::Resolve(const std::string& ip) const {
  return QueryArpTable(ip);
}

std::optional<std::string> MacResolver::QueryArpTable(
    const std::string& ip) const {
#if defined(__APPLE__)
  int mib[6] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_FLAGS, RTF_LLINFO};
  size_t needed = 0;
  if (::sysctl(mib, 6, nullptr, &needed, nullptr, 0) < 0 || needed == 0) {
    return std::nullopt;
  }

  std::string buffer;
  buffer.resize(needed);
  if (::sysctl(mib, 6, buffer.data(), &needed, nullptr, 0) < 0) {
    return std::nullopt;
  }

  in_addr target_addr{};
  if (::inet_pton(AF_INET, ip.c_str(), &target_addr) != 1) {
    return std::nullopt;
  }

  char* ptr = buffer.data();
  char* end = buffer.data() + needed;
  while (ptr < end) {
    auto* rtm = reinterpret_cast<rt_msghdr*>(ptr);
    ptr += rtm->rtm_msglen;
    if (rtm->rtm_addrs == 0) {
      continue;
    }

    sockaddr* sa = reinterpret_cast<sockaddr*>(rtm + 1);
    sockaddr_in* sin = nullptr;
    sockaddr_dl* sdl = nullptr;

    for (int i = 0; i < RTAX_MAX; ++i) {
      if (!(rtm->rtm_addrs & (1 << i))) {
        continue;
      }
      if (sa->sa_family == AF_INET && i == RTAX_DST) {
        sin = reinterpret_cast<sockaddr_in*>(sa);
      } else if (sa->sa_family == AF_LINK && i == RTAX_GATEWAY) {
        sdl = reinterpret_cast<sockaddr_dl*>(sa);
      }

      std::size_t sa_len = sa->sa_len;
      if (sa_len == 0) {
        sa_len = sizeof(sockaddr_in);
      }
      sa = reinterpret_cast<sockaddr*>(reinterpret_cast<char*>(sa) + sa_len);
    }

    if (!sin || !sdl) {
      continue;
    }

    if (sin->sin_addr.s_addr == target_addr.s_addr && sdl->sdl_alen == 6) {
      const unsigned char* mac = reinterpret_cast<const unsigned char*>(
          LLADDR(sdl));
      return FormatMac(mac, 6);
    }
  }

  return std::nullopt;
#else
  ifaddrs* ifaddr = nullptr;
  if (getifaddrs(&ifaddr) != 0) {
    return std::nullopt;
  }

  int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    freeifaddrs(ifaddr);
    return std::nullopt;
  }

  std::optional<std::string> result;
  for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }

    arpreq req{};
    std::memset(&req, 0, sizeof(req));

    auto* pa = reinterpret_cast<sockaddr_in*>(&req.arp_pa);
    pa->sin_family = AF_INET;
    if (::inet_pton(AF_INET, ip.c_str(), &pa->sin_addr) != 1) {
      continue;
    }

    std::strncpy(req.arp_dev, ifa->ifa_name, sizeof(req.arp_dev) - 1);

    if (::ioctl(sock, SIOCGARP, &req) == 0) {
      if (req.arp_ha.sa_family == ARPHRD_ETHER) {
        auto* mac = reinterpret_cast<unsigned char*>(req.arp_ha.sa_data);
        result = FormatMac(mac, 6);
        break;
      }
    }
  }

  ::close(sock);
  freeifaddrs(ifaddr);
  return result;
#endif
}

std::string MacResolver::FormatMac(const unsigned char* data,
                                   std::size_t length) const {
  std::string mac;
  mac.reserve(length * 3);
  for (std::size_t i = 0; i < length; ++i) {
    char buffer[4];
    std::snprintf(buffer, sizeof(buffer), "%02X", data[i]);
    mac.append(buffer);
    if (i + 1 < length) {
      mac.push_back(':');
    }
  }
  return mac;
}

}  // namespace ipcscan
