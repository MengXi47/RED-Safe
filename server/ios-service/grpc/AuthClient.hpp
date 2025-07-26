#pragma once
#include <string>
#include "absl/status/status.h"
#ifndef GRPC_CUSTOM_UTIL_STATUS
#define GRPC_CUSTOM_UTIL_STATUS ::absl::Status
#endif
#include <grpcpp/grpcpp.h>
#include "auth.grpc.pb.h"

namespace redsafe::server::grpc {

class AuthClient {
 public:
  explicit AuthClient(const std::string& address);
  int Decode(const std::string& token, std::string& user_id, std::string& error);
 private:
  std::unique_ptr<::redsafe::grpc::UserAuthService::Stub> stub_;
};
} // namespace redsafe::server::grpc
