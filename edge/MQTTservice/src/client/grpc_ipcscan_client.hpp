#pragma once

#include <memory>
#include <string>

#include "client/ipcscan_client.hpp"
#include "ipcscan.grpc.pb.h"

class GrpcIpcscanClient : public IIpcscanClient {
 public:
  explicit GrpcIpcscanClient(const std::string& target);
  std::string scan() override;

 private:
  std::unique_ptr<::ipcscan::grpc::IPCScanService::Stub> stub_;
};

