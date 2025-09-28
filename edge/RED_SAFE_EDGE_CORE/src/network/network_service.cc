#include "network/network_service.hpp"

#include "common/logging.hpp"

namespace network {

::grpc::Status NetworkConfigGrpcService::GetNetworkConfig(
    ::grpc::ServerContext*,
    const ipcscan::grpc::NetworkInterfaceRequest* request,
    ipcscan::grpc::NetworkConfigResponse* response) {
  NetworkConfig config;
  std::string error;
  if (!FetchNetworkConfig(request->interface(), config, error)) {
    LogErrorFormat("GetNetworkConfig 失敗: {}", error);
    return ::grpc::Status(::grpc::StatusCode::INTERNAL, error);
  }

  auto* cfg = response->mutable_config();
  cfg->set_interface(config.interface);
  cfg->set_ip(config.ip);
  cfg->set_mac(config.mac);
  cfg->set_netmask(config.netmask);
  cfg->set_gateway(config.gateway);
  for (const auto& dns : config.dns_servers) {
    cfg->add_dns_servers(dns);
  }
  return ::grpc::Status::OK;
}

::grpc::Status NetworkConfigGrpcService::SetNetworkConfig(
    ::grpc::ServerContext*,
    const ipcscan::grpc::NetworkConfig* request,
    ipcscan::grpc::SetNetworkConfigResponse* response) {
  NetworkConfig config;
  config.interface = request->interface();
  config.ip = request->ip();
  config.mac = request->mac();
  config.netmask = request->netmask();
  config.gateway = request->gateway();
  config.dns_servers.assign(request->dns_servers().begin(),
                            request->dns_servers().end());

  std::string error;
  if (!ApplyNetworkConfig(config, error)) {
    response->set_success(false);
    response->set_error_message(error);
    LogErrorFormat("SetNetworkConfig 失敗: {}", error);
    return ::grpc::Status::OK;
  }

  response->set_success(true);
  return ::grpc::Status::OK;
}

std::shared_ptr<NetworkConfigGrpcService> MakeNetworkConfigService() {
  return std::make_shared<NetworkConfigGrpcService>();
}

}  // namespace network

