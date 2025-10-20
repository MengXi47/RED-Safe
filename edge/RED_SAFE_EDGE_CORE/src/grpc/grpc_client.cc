#include "grpc/grpc_client.hpp"

#include "util/logging.hpp"

#include <memory>
#include <optional>
#include <string>

#include <grpcpp/grpcpp.h>
#include <iptool.grpc.pb.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using iptool::GetNetworkConfigRequest;
using iptool::GetNetworkConfigResponse;
using iptool::NetworkConfig;
using iptool::NetworkService;

NetworkServiceClient::NetworkServiceClient(
    const std::shared_ptr<Channel>& channel)
    : stub_(NetworkService::NewStub(channel)) {}

std::optional<NetworkConfig> NetworkServiceClient::GetNetworkConfig(
    const std::string& interface_name) const {
  GetNetworkConfigRequest request;
  request.set_interface_name(interface_name);

  GetNetworkConfigResponse response;
  ClientContext context;

  if (const Status status =
          stub_->GetNetworkConfig(&context, request, &response);
      !status.ok()) {
    LogErrorFormat(
        "GetNetworkConfig RPC 失敗，interface={} code={} message={}",
        interface_name,
        static_cast<int>(status.error_code()),
        status.error_message());
    return std::nullopt;
  }

  if (!response.has_config()) {
    LogWarnFormat(
        "GetNetworkConfig 回應未包含設定，interface={}", interface_name);
    return std::nullopt;
  }

  NetworkConfig config;
  config.CopyFrom(response.config());
  return config;
}
