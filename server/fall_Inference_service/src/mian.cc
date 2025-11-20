#include "grpc/server.hpp"

#include <fall_model/inference_adapter.hpp>

#include <grpcpp/grpcpp.h>
#include <folly/init/Init.h>
#include <folly/logging/xlog.h>

#include <exception>
#include <memory>
#include <string>

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  std::string model_path = "fall_probability_model_ts.pt";
  std::string server_address = "0.0.0.0:30050";

  std::shared_ptr<fall_model::InferenceAdapter> adapter;
  try {
    adapter = std::make_shared<fall_model::InferenceAdapter>(model_path);
  } catch (const std::exception& ex) {
    XLOGF(ERR, "failed to load model from {}: {}", model_path, ex.what());
    return 1;
  }

  fallinference::FallInferenceServiceImpl service(adapter);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    XLOGF(ERR, "failed to start gRPC server on {}", server_address);
    return 1;
  }

  XLOGF(
      INFO,
      "FallInferenceService gRPC server listening on {} with model : {}",
      server_address,
      model_path);

  server->Wait();
  return 0;
}
