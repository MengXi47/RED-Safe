#pragma once

#include <FallService.grpc.pb.h>

#include <array>
#include <memory>
#include <mutex>

namespace fall_model {
class InferenceAdapter;
}  // namespace fall_model

namespace fallinference {

class FallInferenceServiceImpl final : public FallInferenceService::Service {
 public:
  explicit FallInferenceServiceImpl(
      std::shared_ptr<fall_model::InferenceAdapter> adapter);

  grpc::Status InferFallProbability(grpc::ServerContext* context,
                                    const FallInferenceRequest* request,
                                    FallInferenceResponse* response) override;

 private:
  std::shared_ptr<fall_model::InferenceAdapter> adapter_;
  std::mutex infer_mutex_;
};

}  // namespace fallinference
