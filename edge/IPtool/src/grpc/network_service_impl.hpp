#pragma once

#include <grpcpp/grpcpp.h>

#include "iptool.grpc.pb.h"
#include "network/network_service.hpp"

namespace iptool::grpcservice {

class NetworkServiceGrpc final : public iptool::NetworkService::Service {
 public:
  explicit NetworkServiceGrpc();
  ~NetworkServiceGrpc() override = default;

  grpc::Status GetNetworkConfig(
      grpc::ServerContext* context,
      const iptool::GetNetworkConfigRequest* request,
      iptool::GetNetworkConfigResponse* response) override;

  grpc::Status UpdateNetworkConfig(
      grpc::ServerContext* context,
      const iptool::UpdateNetworkConfigRequest* request,
      iptool::UpdateNetworkConfigResponse* response) override;

  grpc::Status SwitchToDhcp(
      grpc::ServerContext* context,
      const iptool::SwitchToDhcpRequest* request,
      iptool::SwitchToDhcpResponse* response) override;

 private:
  static void FillProtoConfig(
      const NetworkConfigData& data, iptool::NetworkConfig* proto_config);
};

} // namespace iptool::grpcservice
