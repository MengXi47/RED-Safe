#pragma once

#include "ipcscan/app/ipc_scanner.hpp"
#include "ipcscan/core/device_info.hpp"
#include "ipcscan/network/mac_resolver.hpp"
#include "ipcscan/network/ws_discovery_client.hpp"
#include "ipcscan/parser/onvif_response_parser.hpp"

#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace ipcscan {

// 封裝 IPCscan 掃描流程的執行單元
class ScanExecutor {
 public:
  // 建構子：設定單次掃描的逾時時間
  explicit ScanExecutor(std::chrono::milliseconds timeout);

  // 執行掃描並回傳 JSON 陣列字串（至少為 []）
  std::string RunScan();

 private:
  // 將掃描結果轉成 JSON 字串
  static std::string BuildJson(const std::vector<DeviceInfo>& devices) ;

  OnvifResponseParser parser_;
  MacResolver mac_resolver_;
  IpcScanner scanner_;
  std::chrono::milliseconds timeout_;
  std::mutex mutex_;
};

} // namespace ipcscan
