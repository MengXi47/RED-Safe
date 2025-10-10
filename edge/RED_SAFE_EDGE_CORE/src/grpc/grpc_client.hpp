#pragma once

#include <memory>
#include <optional>
#include <string>

#include <grpcpp/grpcpp.h>

#include "iptool.grpc.pb.h"

class NetworkServiceClient {
 public:
  explicit NetworkServiceClient(const std::shared_ptr<grpc::Channel>& channel);

  // 呼叫 gRPC 的 GetNetworkConfig，取得指定介面的完整設定
  [[nodiscard]] std::optional<iptool::NetworkConfig> GetNetworkConfig(
      const std::string& interface_name) const;

 private:
  std::unique_ptr<iptool::NetworkService::Stub> stub_;
};
