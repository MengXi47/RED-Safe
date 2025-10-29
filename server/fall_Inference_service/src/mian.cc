#include "grpc/server.hpp"

#include <fall_model/inference_adapter.hpp>

#include <folly/init/Init.h>
#include <grpcpp/grpcpp.h>

#include <exception>
#include <iostream>
#include <memory>
#include <string>

namespace {

constexpr int kExpectedFeatureCount = 9;
constexpr const char* kDefaultAddress = "0.0.0.0:30050";
constexpr const char* kDefaultModelPath = "fall_probability_model_ts.pt";

}  // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  std::string model_path = kDefaultModelPath;
  std::string server_address = kDefaultAddress;

  if (argc > 1 && argv[1]) {
    model_path = argv[1];
  }
  if (argc > 2 && argv[2]) {
    server_address = argv[2];
  }

  std::shared_ptr<fall_model::InferenceAdapter> adapter;
  try {
    adapter = std::make_shared<fall_model::InferenceAdapter>(model_path);
  } catch (const std::exception& ex) {
    std::cerr << "[ERR] failed to load model from " << model_path << ": "
              << ex.what() << "\n";
    return 1;
  }

  fallinference::FallInferenceServiceImpl service(adapter);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    std::cerr << "[ERR] failed to start gRPC server on " << server_address
              << "\n";
    return 1;
  }

  std::cout << "FallInferenceService gRPC server listening on "
            << server_address << " with model " << model_path << "\n";
  std::cout << "Expecting " << kExpectedFeatureCount
            << " features per request.\n";

  server->Wait();
  return 0;
}
