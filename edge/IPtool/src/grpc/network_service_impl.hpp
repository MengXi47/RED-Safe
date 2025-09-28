#pragma once

#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "app/network_query_service.hpp"
#include "app/network_update_service.hpp"
#include "domain/network_error.hpp"
#include "iptool.grpc.pb.h"

namespace iptool::grpcservice {

class NetworkServiceImpl final : public ::iptool::NetworkService::Service {
 public:
  NetworkServiceImpl(std::shared_ptr<app::NetworkQueryService> query_service,
                     std::shared_ptr<app::NetworkUpdateService> update_service);

  ::grpc::Status GetNetworkConfig(::grpc::ServerContext* context,
                                  const ::iptool::GetNetworkConfigRequest* request,
                                  ::iptool::GetNetworkConfigResponse* response) override;

  ::grpc::Status UpdateNetworkConfig(
      ::grpc::ServerContext* context,
      const ::iptool::UpdateNetworkConfigRequest* request,
      ::iptool::UpdateNetworkConfigResponse* response) override;

 private:
  static domain::NetworkConfig FromProto(const ::iptool::NetworkConfig& proto);
  static void ToProto(const domain::NetworkConfig& config,
                      ::iptool::NetworkConfig* proto);

  std::shared_ptr<app::NetworkQueryService> query_service_;
  std::shared_ptr<app::NetworkUpdateService> update_service_;
};

}  // namespace iptool::grpcservice
