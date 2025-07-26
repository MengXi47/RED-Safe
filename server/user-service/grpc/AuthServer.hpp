#pragma once
#include <memory>
#include "absl/status/status.h"

using absl::Status;
#ifndef GRPC_CUSTOM_UTIL_STATUS
#define GRPC_CUSTOM_UTIL_STATUS ::absl::Status
#endif
#include <string>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "auth.grpc.pb.h"

namespace redsafe::server::grpc {

class AuthServer {
 public:
  AuthServer();
  void Run(const std::string& address);

 private:
  class ServiceImpl;
  std::unique_ptr<std::thread> server_thread_;
};

} // namespace redsafe::server::grpc
