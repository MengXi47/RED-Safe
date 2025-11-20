#include <memory>
#include <print>
#include <string>

#include <grpcpp/grpcpp.h>

#include "grpc/network_service_impl.hpp"
#include "util/logging.hpp"

int main(int argc, char* argv[]) {
  std::string server_address = "0.0.0.0:20002";

  iptool::grpcservice::NetworkServiceGrpc service{};
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  const auto server = builder.BuildAndStart();
  if (!server) {
    LogError("無法啟動 gRPC 伺服器");
    return 1;
  }

  LogInfoFormat("gRPC 伺服器啟動於 {}", server_address);
  LogInfo("等待客戶端請求中...");
  server->Wait();
  return 0;
}
