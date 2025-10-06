#include "network_info_handler.hpp"

#include "grpc/grpc_client.hpp"
#include "util/logging.hpp"

using boost::asio::awaitable;

NetworkInfoHandler::NetworkInfoHandler(EdgeConfig& config) : config_(config) {}

awaitable<void> NetworkInfoHandler::Handle(const CommandMessage&) {
  const std::string interface_name =
      config_.network_interface.empty() ? "eth0" : config_.network_interface;

  const NetworkServiceClient client(
      grpc::CreateChannel(
          config_.iptool_target, grpc::InsecureChannelCredentials()));

  if (auto network_config = client.GetNetworkConfig(interface_name);
      !network_config) {
    LogErrorFormat(
        "查詢網路設定失敗，target={} interface={}",
        config_.iptool_target,
        interface_name);
  } else {
    const auto& net = *network_config;
    LogInfoFormat(
        "介面 {} IP {} Gateway {} Subnet {}",
        net.interface_name(),
        net.ip_address(),
        net.gateway(),
        net.subnet_mask());
    for (const auto& dns : net.dns()) {
      LogInfoFormat("DNS: {}", dns);
    }
  }

  co_return;
}
