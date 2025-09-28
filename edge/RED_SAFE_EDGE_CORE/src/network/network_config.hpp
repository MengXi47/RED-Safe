#pragma once

#include <string>
#include <vector>

namespace network {

struct NetworkConfig {
  std::string interface;
  std::string ip;
  std::string mac;
  std::string netmask;
  std::string gateway;
  std::vector<std::string> dns_servers;
};

// 取得網路卡設定，若 interface 為空會選擇第一個非 loopback
// 成功回傳 true 並填入 config，失敗則回傳 false 並填入 error 訊息
bool FetchNetworkConfig(const std::string& interface,
                        NetworkConfig& config,
                        std::string& error);

// 套用網路卡設定（需具備對應系統權限）
bool ApplyNetworkConfig(const NetworkConfig& config,
                        std::string& error);

}

