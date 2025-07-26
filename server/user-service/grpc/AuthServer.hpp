#pragma once
#include <memory>
#include <string>
#include <thread>
#include "absl/status/status.h"
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
