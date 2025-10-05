#include "grpc/ip_resolver.hpp"

#include "common/logging.hpp"
#include "grpc/grpc_client.hpp"

#include <grpcpp/grpcpp.h>

#include <optional>
#include <string>

namespace {
constexpr const char* kDefaultInterface = "eth0";
}

std::optional<std::string> FetchEdgeIpFromIptool(const EdgeConfig& config) {
  const std::string target =
      config.iptool_target.empty() ? "localhost:20002" : config.iptool_target;

  if (target.empty()) {
    LogWarn("IPtool 目標位址未設定，無法取得 Edge IP");
    return std::nullopt;
  }

  const std::string interface_name = config.network_interface.empty()
      ? std::string{kDefaultInterface}
      : config.network_interface;

  if (interface_name.empty()) {
    LogWarn("未指定 Edge 網路介面，無法取得 IP");
    return std::nullopt;
  }

  auto channel =
      grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
  NetworkServiceClient client(channel);

  auto network_config = client.GetNetworkConfig(interface_name);
  if (!network_config) {
    LogWarnFormat(
        "GetNetworkConfig 失敗，target={} interface={}",
        target,
        interface_name);
    return std::nullopt;
  }

  const auto& config_proto = *network_config;
  if (config_proto.ip_address().empty()) {
    LogWarnFormat(
        "IPtool 回傳空 IP，target={} interface={}", target, interface_name);
    return std::nullopt;
  }

  return config_proto.ip_address();
}
