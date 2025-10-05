#include "grpc/network_service_impl.hpp"

#include "util/logging.hpp"

namespace iptool::grpcservice {

void NetworkServiceGrpc::FillProtoConfig(
    const NetworkConfigData& data, iptool::NetworkConfig* proto_config) {
  proto_config->set_interface_name(data.interface_name);
  proto_config->set_ip_address(data.ip_address);
  proto_config->set_subnet_mask(data.subnet_mask);
  proto_config->set_gateway(data.gateway);
  proto_config->set_dns(data.dns);
  proto_config->set_mode(data.mode);
}

NetworkServiceGrpc::NetworkServiceGrpc() = default;

grpc::Status NetworkServiceGrpc::GetNetworkConfig(
    grpc::ServerContext* /*context*/,
    const iptool::GetNetworkConfigRequest* request,
    iptool::GetNetworkConfigResponse* response) {
  LogInfoFormat("收到 GetNetworkConfig 請求: {}", request->interface_name());
  auto config = ::NetworkService::GetNetworkConfig(request->interface_name());
  if (!config) {
    return {grpc::StatusCode::NOT_FOUND, "無法取得指定介面的設定"};
  }
  FillProtoConfig(*config, response->mutable_config());
  return grpc::Status::OK;
}

grpc::Status NetworkServiceGrpc::UpdateNetworkConfig(
    grpc::ServerContext* /*context*/,
    const iptool::UpdateNetworkConfigRequest* request,
    iptool::UpdateNetworkConfigResponse* response) {
  const auto& input = request->config();
  LogInfoFormat("收到 UpdateNetworkConfig 請求: {}", input.interface_name());

  if (input.mode() != iptool::NETWORK_MODE_MANUAL) {
    response->set_success(false);
    response->set_message("請指定 NETWORK_MODE_MANUAL");
    return grpc::Status::OK;
  }

  NetworkConfigData config;
  config.interface_name = input.interface_name();
  config.ip_address = input.ip_address();
  config.subnet_mask = input.subnet_mask();
  config.gateway = input.gateway();
  config.dns = input.dns();
  config.mode = input.mode();

  auto result = ::NetworkService::SetManualConfig(config);
  response->set_success(result.success);
  response->set_message(result.message);
  return grpc::Status::OK;
}

grpc::Status NetworkServiceGrpc::SwitchToDhcp(
    grpc::ServerContext* /*context*/,
    const iptool::SwitchToDhcpRequest* request,
    iptool::SwitchToDhcpResponse* response) {
  LogInfoFormat("收到 SwitchToDhcp 請求: {}", request->interface_name());
  auto result = ::NetworkService::SwitchToDhcp(request->interface_name());
  response->set_success(result.success);
  response->set_message(result.message);
  return grpc::Status::OK;
}

} // namespace iptool::grpcservice
