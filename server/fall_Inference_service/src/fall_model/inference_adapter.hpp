#pragma once

#include <array>
#include <memory>
#include <string>

namespace fall_model {

/**
 * Lightweight façade that hides the Torch dependency behind a PIMPL so callers
 * do not need to include libtorch headers (which conflict with the protobuf
 * version used by gRPC).
 */
class InferenceAdapter {
 public:
  explicit InferenceAdapter(const std::string& model_path);
  ~InferenceAdapter();

  InferenceAdapter(InferenceAdapter&&) noexcept;
  InferenceAdapter& operator=(InferenceAdapter&&) noexcept;
  InferenceAdapter(const InferenceAdapter&) = delete;
  InferenceAdapter& operator=(const InferenceAdapter&) = delete;

  float infer_one(const std::array<float, 9>& features);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace fall_model
