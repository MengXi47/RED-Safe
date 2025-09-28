#include "app/iptool_application.hpp"

#include <stdexcept>

#include <grpcpp/server_builder.h>

#include "common/logging.hpp"
#include "grpc/network_service_impl.hpp"

namespace iptool::app {

IpToolApplication::IpToolApplication(
    std::shared_ptr<NetworkQueryService> query_service,
    std::shared_ptr<NetworkUpdateService> update_service,
    std::string server_address)
    : query_service_(std::move(query_service)),
      update_service_(std::move(update_service)),
      server_address_(std::move(server_address)) {}

int IpToolApplication::Run() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());

  grpcservice::NetworkServiceImpl service(query_service_, update_service_);
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    throw std::runtime_error("Failed to start gRPC server");
  }

  LogInfoFormat("IPtool gRPC server listening on {}", server_address_);
  server->Wait();
  LogInfo("IPtool gRPC server stopped");
  return EXIT_SUCCESS;
}

}  // namespace iptool::app
