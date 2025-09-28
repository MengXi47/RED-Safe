#pragma once

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "iptool.grpc.pb.h"

class NetworkServiceClient {
 public:
  explicit NetworkServiceClient(std::shared_ptr<grpc::Channel> channel);

  // 呼叫 gRPC 的 GetNetworkConfig
  void GetNetworkConfig(const std::string& interface_name) const;

 private:
  std::unique_ptr<iptool::NetworkService::Stub> stub_;
};