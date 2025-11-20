#pragma once

#include <optional>
#include <string>

#include "iptool.pb.h"

struct NetworkConfigData {
  std::string interface_name;
  std::string ip_address;
  std::string subnet_mask;
  std::string gateway;
  std::string dns;
  iptool::NetworkMode mode = iptool::NETWORK_MODE_UNSPECIFIED;
};

struct OperationResult {
  bool success = false;
  std::string message;
};

class NetworkService {
 public:
  NetworkService() = default;
  ~NetworkService() = default;
  NetworkService(NetworkService const&) = delete;
  NetworkService& operator=(NetworkService const&) = delete;

  static std::optional<NetworkConfigData> GetNetworkConfig(
      const std::string& interface_name);
  static OperationResult SetManualConfig(const NetworkConfigData& config);
  static OperationResult SwitchToDhcp(const std::string& interface_name);
};
