#include "ipc_scanner.hpp"

namespace ipcscan {

IpcScanner::IpcScanner(
    const IResponseParser& parser, const IMacResolver& mac_resolver)
    : parser_(parser), mac_resolver_(mac_resolver) {}

std::vector<DeviceInfo> IpcScanner::Scan(IDiscoveryTransport& transport) const {
  std::unordered_map<std::string, DeviceInfo> devices;

  // 透過WS-Discovery取得所有Probe回應
  for (auto&& response : transport.Probe()) {
    auto& device = devices[response.first];
    device.ip = response.first;
    // 解析Scopes資訊更新MAC與名稱
    parser_.Parse(response.second, device);
  }

  // 若回應內缺少資訊，改由ARP查找MAC並補上預設名稱
  for (auto& [ip, device] : devices) {
    if (device.mac.empty()) {
      if (auto mac = mac_resolver_.Resolve(ip)) {
        device.mac = *mac;
      }
    }
    if (device.name.empty()) {
      device.name = "IPC";
    }
  }

  std::vector<DeviceInfo> result;
  result.reserve(devices.size());
  for (auto& [ip, device] : devices) {
    result.push_back(std::move(device));
  }
  return result;
}

} // namespace ipcscan
