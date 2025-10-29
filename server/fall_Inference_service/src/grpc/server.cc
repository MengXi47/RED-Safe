#include "server.hpp"

#include <fall_model/inference_adapter.hpp>

#include <array>
#include <exception>
#include <mutex>
#include <string>

namespace fallinference {

FallInferenceServiceImpl::FallInferenceServiceImpl(
    std::shared_ptr<fall_model::InferenceAdapter> adapter)
    : adapter_(std::move(adapter)) {}

grpc::Status FallInferenceServiceImpl::InferFallProbability(
    grpc::ServerContext* /*context*/, const FallInferenceRequest* request,
    FallInferenceResponse* response) {
  if (!request) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "request payload is null");
  }
  if (!response) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "response container is null");
  }
  if (!adapter_) {
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION,
                        "inference backend not initialised");
  }

  if (request->features_size() != 9) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "expected exactly 9 features");
  }

  std::array<float, 9> features{};
  for (int i = 0; i < 9; ++i) {
    features[static_cast<size_t>(i)] = request->features(i);
  }

  try {
    std::lock_guard<std::mutex> guard(infer_mutex_);
    const float probability = adapter_->infer_one(features);
    response->set_probability(static_cast<double>(probability));
  } catch (const std::exception& ex) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        std::string("model inference failed: ") + ex.what());
  }

  return grpc::Status::OK;
}

}  // namespace fallinference
