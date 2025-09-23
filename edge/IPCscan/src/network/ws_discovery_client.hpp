#pragma once

#include "network/discovery_transport.hpp"
#include "network/udp_socket.hpp"

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace ipcscan {

class WsDiscoveryClient : public IDiscoveryTransport {
 public:
  // 建構子負責初始化UDP socket設定
  explicit WsDiscoveryClient(std::chrono::milliseconds timeout);

  // 發送Probe並取得所有回應
  std::vector<std::pair<std::string, std::string>> Probe() override;

 private:
  // 建立符合ONVIF規範的Probe訊息
  std::string BuildProbeMessage() const;

  UdpSocket socket_;
  std::chrono::milliseconds timeout_;
};

}  // namespace ipcscan
