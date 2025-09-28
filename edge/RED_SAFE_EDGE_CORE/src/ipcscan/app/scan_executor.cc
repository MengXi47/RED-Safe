#include "ipcscan/app/scan_executor.hpp"

#include <nlohmann/json.hpp>

namespace ipcscan {

// 建構子：設定掃描逾時並初始化依賴
ScanExecutor::ScanExecutor(std::chrono::milliseconds timeout)
    : parser_(),
      mac_resolver_(),
      scanner_(parser_, mac_resolver_),
      timeout_(timeout) {}

// 執行一次 IPC 掃描並回傳 JSON 字串
std::string ScanExecutor::RunScan() {
  std::lock_guard<std::mutex> lock(mutex_);
  WsDiscoveryClient transport(timeout_);
  auto devices = scanner_.Scan(transport);
  return BuildJson(devices);
}

// 將掃描結果轉成 JSON 陣列格式
std::string ScanExecutor::BuildJson(
    const std::vector<DeviceInfo>& devices) const {
  nlohmann::json array = nlohmann::json::array();
  for (const auto& device : devices) {
    nlohmann::json entry{
        {"ip", device.ip}, {"mac", device.mac}, {"name", device.name}};
    array.push_back(std::move(entry));
  }
  return array.dump();
}

} // namespace ipcscan
