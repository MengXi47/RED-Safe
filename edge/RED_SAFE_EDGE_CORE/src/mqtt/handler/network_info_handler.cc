#include "mqtt/handler/network_info_handler.hpp"

#include <iptool.pb.h>
#include "grpc/grpc_client.hpp"
#include "util/logging.hpp"

#include <folly/dynamic.h>

#include <string>
#include <utility>

using boost::asio::awaitable;

namespace {

std::string ModeToString(iptool::NetworkMode mode) {
  switch (mode) {
    case iptool::NETWORK_MODE_DHCP:
      return "dhcp";
    case iptool::NETWORK_MODE_MANUAL:
      return "manual";
    default:
      return "unspecified";
  }
}

folly::dynamic BuildNetworkInfoResult(const iptool::NetworkConfig& net) {
  folly::dynamic result = folly::dynamic::object;
  result["interface_name"] = net.interface_name();
  result["ip_address"] = net.ip_address();
  result["subnet_mask"] = net.subnet_mask();
  result["gateway"] = net.gateway();
  result["dns"] = net.dns();

  folly::dynamic mode = folly::dynamic::object;
  mode["name"] = ModeToString(net.mode());
  mode["value"] = static_cast<int>(net.mode());
  mode["raw"] = iptool::NetworkMode_Name(net.mode());
  result["mode"] = std::move(mode);
  return result;
}

} // namespace

NetworkInfoHandler::NetworkInfoHandler(
    EdgeConfig& config, CommandPublishFn publish_response)
    : publish_response_(std::move(publish_response)), config_(config) {}

awaitable<void> NetworkInfoHandler::Handle(const CommandMessage& command) {
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
    const std::string response =
        BuildErrorResponse(command.trace_id, command.code, "查詢網路設定失敗");
    co_await publish_response_(response, "發佈網路設定查詢結果失敗");
  } else {
    const auto& net = *network_config;
    LogInfoFormat(
        "介面 {} IP {} Gateway {} Subnet {}",
        net.interface_name(),
        net.ip_address(),
        net.gateway(),
        net.subnet_mask());
    LogInfoFormat("DNS: {}", net.dns());
    LogInfoFormat(
        "網路模式 mode={} ({})",
        ModeToString(net.mode()),
        iptool::NetworkMode_Name(net.mode()));

    auto result = BuildNetworkInfoResult(net);
    const std::string response =
        BuildSuccessResponse(command.trace_id, command.code, std::move(result));
    const bool published =
        co_await publish_response_(response, "發佈網路設定查詢結果失敗");
    if (published) {
      LogInfoFormat(
          "網路設定結果已送出 trace_id={} interface={}",
          command.trace_id,
          interface_name);
    }
  }

  co_return;
}
