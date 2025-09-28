#pragma once

#include "mac_resolver_port.hpp"

#include <optional>
#include <string>

namespace ipcscan {

class MacResolver : public IMacResolver {
 public:
  // 透過ARP快取查詢指定IP的MAC地址
  [[nodiscard]] std::optional<std::string> Resolve(const std::string& ip) const override;

 private:
  // 讀取ARP快取資料
  [[nodiscard]] std::optional<std::string> QueryArpTable(const std::string& ip) const;
  // 轉換位元組為MAC格式
  std::string FormatMac(const unsigned char* data, std::size_t length) const;
};

} // namespace ipcscan
