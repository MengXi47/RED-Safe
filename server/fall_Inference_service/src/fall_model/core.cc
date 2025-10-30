#include "core.hpp"
#include <stdexcept>

torch::Device FallProbInfer::selectDevice() {
#if defined(TORCH_CUDA_AVAILABLE)
  // 若使用的是 CUDA 版 libtorch，才會提供 torch::cuda::is_available()
  if (torch::cuda::is_available()) {
    return torch::Device(torch::kCUDA, 0);
  }
#endif
  // macOS 上的 libtorch 通常是 CPU 版，預設回 CPU
  return torch::kCPU;
}

FallProbInfer::FallProbInfer(const std::string& ts_model_path) {
  try {
    // 先用 CPU 讀檔最穩，之後再搬到 GPU（若有）
    module_ = torch::jit::load(ts_model_path, torch::kCPU);
    module_.eval();
  } catch (const c10::Error& e) {
    throw std::runtime_error(
        std::string("Load TorchScript failed: ") + e.what());
  }
  device_ = selectDevice();
  if (device_.is_cuda())
    module_.to(device_);
}

torch::Tensor FallProbInfer::toTensor(
    const std::vector<std::array<float, 9>>& batch) {
  const auto n = static_cast<int64_t>(batch.size());
  torch::Tensor t =
      torch::empty({n, 9}, torch::TensorOptions().dtype(torch::kFloat32));
  // 逐行拷貝，避免 from_blob 的生命週期/對齊問題
  for (int64_t i = 0; i < n; ++i) {
    auto row =
        torch::from_blob((void*)batch[i].data(), {9}, torch::kFloat32).clone();
    t.index_put_({i}, row);
  }
  return t.to(device_);
}

float FallProbInfer::inferOne(const std::array<float, 9>& features) {
  std::vector<std::array<float, 9>> batch{features};
  auto probs = inferBatch(batch);
  return probs.front();
}

std::vector<float> FallProbInfer::inferBatch(
    const std::vector<std::array<float, 9>>& batch) {
  if (batch.empty())
    return {};
  torch::NoGradGuard no_grad;

  auto input = toTensor(batch);
  std::vector<torch::jit::IValue> args;
  args.emplace_back(input);
  auto logits = module_.forward(args).toTensor();
  auto probs = torch::sigmoid(logits) * 100.0f; // -> 百分比

  probs = probs.to(torch::kCPU);
  std::vector<float> out;
  out.reserve(batch.size());
  // logits shape: [N, 1]，把每列取出
  for (int64_t i = 0; i < probs.size(0); ++i) {
    out.push_back(probs[i][0].item<float>());
  }
  return out;
}