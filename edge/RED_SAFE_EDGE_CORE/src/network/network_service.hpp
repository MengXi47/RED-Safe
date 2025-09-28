#pragma once

#include "network/network_config.hpp"

#include "red_safe_edge_core.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <memory>

namespace network {

class NetworkConfigGrpcService final
    : public ipcscan::grpc::NetworkConfigService::Service {
 public:
  ::grpc::Status GetNetworkConfig(::grpc::ServerContext* context,
                                  const ipcscan::grpc::NetworkInterfaceRequest* request,
                                  ipcscan::grpc::NetworkConfigResponse* response) override;

  ::grpc::Status SetNetworkConfig(::grpc::ServerContext* context,
                                  const ipcscan::grpc::NetworkConfig* request,
                                  ipcscan::grpc::SetNetworkConfigResponse* response) override;
};

std::shared_ptr<NetworkConfigGrpcService> MakeNetworkConfigService();

}  // namespace network

