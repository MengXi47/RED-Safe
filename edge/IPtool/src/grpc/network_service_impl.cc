#include "grpc/network_service_impl.hpp"

#include <string>

#include "common/logging.hpp"

namespace iptool::grpcservice {

namespace {
::grpc::Status NetworkErrorStatus(const domain::NetworkError& error) {
  if (dynamic_cast<const domain::NetworkInterfaceNotFoundError*>(&error) !=
      nullptr) {
    return {::grpc::StatusCode::NOT_FOUND, error.what()};
  }
  if (dynamic_cast<const domain::UnsupportedPlatformError*>(&error) !=
      nullptr) {
    return {::grpc::StatusCode::UNIMPLEMENTED, error.what()};
  }
  if (dynamic_cast<const domain::NetworkCommandError*>(&error) != nullptr) {
    return {::grpc::StatusCode::INTERNAL, error.what()};
  }
  return {::grpc::StatusCode::UNKNOWN, error.what()};
}
}  // namespace

NetworkServiceImpl::NetworkServiceImpl(
    std::shared_ptr<app::NetworkQueryService> query_service,
    std::shared_ptr<app::NetworkUpdateService> update_service)
    : query_service_(std::move(query_service)),
      update_service_(std::move(update_service)) {}

::grpc::Status NetworkServiceImpl::GetNetworkConfig(
    ::grpc::ServerContext* /*context*/,
    const ::iptool::GetNetworkConfigRequest* request,
    ::iptool::GetNetworkConfigResponse* response) {
  const std::string interface_name = request->interface_name();
  LogInfoFormat("GetNetworkConfig request interface='{}'", interface_name);
  try {
    const auto config = query_service_->Fetch(interface_name);
    ToProto(config, response->mutable_config());
    LogInfoFormat("GetNetworkConfig success interface='{}'", config.interface_name);
    return ::grpc::Status::OK;
  } catch (const domain::NetworkError& error) {
    LogErrorFormat("GetNetworkConfig failed: {}", error.what());
    return NetworkErrorStatus(error);
  } catch (const std::exception& ex) {
    LogErrorFormat("GetNetworkConfig unexpected failure: {}", ex.what());
    return {::grpc::StatusCode::UNKNOWN, ex.what()};
  }
}

::grpc::Status NetworkServiceImpl::UpdateNetworkConfig(
    ::grpc::ServerContext* /*context*/,
    const ::iptool::UpdateNetworkConfigRequest* request,
    ::iptool::UpdateNetworkConfigResponse* response) {
  const auto& incoming = request->config();
  LogInfoFormat("UpdateNetworkConfig request interface='{}'", incoming.interface_name());
  try {
    update_service_->Apply(FromProto(incoming));
    response->set_success(true);
    response->set_message("Configuration updated");
    LogInfo("UpdateNetworkConfig success");
    return ::grpc::Status::OK;
  } catch (const domain::NetworkError& error) {
    LogErrorFormat("UpdateNetworkConfig failed: {}", error.what());
    response->set_success(false);
    response->set_message(error.what());
    return NetworkErrorStatus(error);
  } catch (const std::exception& ex) {
    LogErrorFormat("UpdateNetworkConfig unexpected failure: {}", ex.what());
    response->set_success(false);
    response->set_message(ex.what());
    return {::grpc::StatusCode::UNKNOWN, ex.what()};
  }
}

domain::NetworkConfig NetworkServiceImpl::FromProto(
    const ::iptool::NetworkConfig& proto) {
  domain::NetworkConfig config;
  config.interface_name = proto.interface_name();
  config.ip_address = proto.ip_address();
  config.mac_address = proto.mac_address();
  config.gateway = proto.gateway();
  config.subnet_mask = proto.subnet_mask();
  config.dns_servers.assign(proto.dns_servers().begin(),
                            proto.dns_servers().end());
  return config;
}

void NetworkServiceImpl::ToProto(const domain::NetworkConfig& config,
                                 ::iptool::NetworkConfig* proto) {
  proto->set_interface_name(config.interface_name);
  proto->set_ip_address(config.ip_address);
  proto->set_mac_address(config.mac_address);
  proto->set_gateway(config.gateway);
  proto->set_subnet_mask(config.subnet_mask);
  proto->clear_dns_servers();
  for (const auto& dns : config.dns_servers) {
    proto->add_dns_servers(dns);
  }
}

}  // namespace iptool::grpcservice
