#include "inference_adapter.hpp"

#include "core.hpp"

namespace fall_model {

class InferenceAdapter::Impl {
 public:
  explicit Impl(const std::string& model_path) : infer_(model_path) {}

  float infer_one(const std::array<float, 9>& features) {
    return infer_.inferOne(features);
  }

 private:
  FallProbInfer infer_;
};

InferenceAdapter::InferenceAdapter(const std::string& model_path)
    : impl_(std::make_unique<Impl>(model_path)) {}

InferenceAdapter::~InferenceAdapter() = default;

InferenceAdapter::InferenceAdapter(InferenceAdapter&&) noexcept = default;
InferenceAdapter& InferenceAdapter::operator=(InferenceAdapter&&) noexcept =
    default;

float InferenceAdapter::infer_one(const std::array<float, 9>& features) {
  return impl_->infer_one(features);
}

} // namespace fall_model
