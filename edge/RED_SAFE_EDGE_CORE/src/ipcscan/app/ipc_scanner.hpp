#pragma once

#include "ipcscan/core/device_info.hpp"
#include "ipcscan/network/discovery_transport.hpp"
#include "ipcscan/network/mac_resolver_port.hpp"
#include "ipcscan/parser/response_parser.hpp"

#include <unordered_map>
#include <vector>

namespace ipcscan {

class IpcScanner {
 public:
  // 透過建構子注入必要的依賴物件
  IpcScanner(const IResponseParser& parser, const IMacResolver& mac_resolver);

  // 執行完整掃描流程並回傳裝置列表
  std::vector<DeviceInfo> Scan(IDiscoveryTransport& transport) const;

 private:
  const IResponseParser& parser_;
  const IMacResolver& mac_resolver_;
};

} // namespace ipcscan
