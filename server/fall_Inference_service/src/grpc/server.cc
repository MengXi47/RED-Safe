#include "server.hpp"

#include <fall_model/inference_adapter.hpp>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/String.h>
#include <folly/logging/xlog.h>

#include <array>
#include <exception>
#include <mutex>
#include <string>

namespace fallinference {

FallInferenceServiceImpl::FallInferenceServiceImpl(
    std::shared_ptr<fall_model::InferenceAdapter> adapter)
    : adapter_(std::move(adapter)) {}

grpc::Status FallInferenceServiceImpl::InferFallProbability(
    grpc::ServerContext* /*context*/,
    const FallInferenceRequest* request,
    FallInferenceResponse* response) {
  if (!request) {
    return grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT, "request payload is null");
  }
  if (!response) {
    return grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT, "response container is null");
  }
  if (!adapter_) {
    return grpc::Status(
        grpc::StatusCode::FAILED_PRECONDITION,
        "inference backend not initialised");
  }

  if (request->features_size() != 9) {
    return grpc::Status(
        grpc::StatusCode::INVALID_ARGUMENT, "expected exactly 9 features");
  }

  std::array<float, 9> features{};
  for (int i = 0; i < 9; ++i) {
    features[static_cast<size_t>(i)] = request->features(i);
  }

  try {
    std::lock_guard<std::mutex> guard(infer_mutex_);
    const float probability = adapter_->infer_one(features);
    const double rounded_probability =
        std::round(probability * 1000.0) / 1000.0;
    XLOGF(
        INFO,
        "probability_raw: {} probability: {}",
        probability,
        rounded_probability);
    response->set_probability(rounded_probability);
  } catch (const std::exception& ex) {
    return grpc::Status(
        grpc::StatusCode::INTERNAL,
        std::string("model inference failed: ") + ex.what());
  }

  return grpc::Status::OK;
}

} // namespace fallinference
