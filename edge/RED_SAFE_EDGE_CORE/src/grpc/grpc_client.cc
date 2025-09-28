#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "grpc_client.hpp"
#include "iptool.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using iptool::GetNetworkConfigRequest;
using iptool::GetNetworkConfigResponse;
using iptool::NetworkService;

NetworkServiceClient::NetworkServiceClient(std::shared_ptr<Channel> channel)
    : stub_(NetworkService::NewStub(channel)) {}

void NetworkServiceClient::GetNetworkConfig(
    const std::string& interface_name) const {
  // 建立請求
  GetNetworkConfigRequest request;
  request.set_interface_name(interface_name);

  // 回應物件
  GetNetworkConfigResponse response;

  // Context
  ClientContext context;

  // 呼叫 RPC
  const Status status = stub_->GetNetworkConfig(&context, request, &response);

  if (status.ok()) {
    const auto& config = response.config();
    std::cout << "Interface: " << config.interface_name() << std::endl;
    std::cout << "IP Address: " << config.ip_address() << std::endl;
    std::cout << "MAC Address: " << config.mac_address() << std::endl;
    std::cout << "Gateway: " << config.gateway() << std::endl;
    std::cout << "Subnet Mask: " << config.subnet_mask() << std::endl;

    std::cout << "DNS Servers:" << std::endl;
    for (const auto& dns : config.dns_servers()) {
      std::cout << "  - " << dns << std::endl;
    }
  } else {
    std::cerr << "RPC failed: " << status.error_code() << " - "
              << status.error_message() << std::endl;
  }
}
