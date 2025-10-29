#pragma once
#include <torch/script.h>
#include <array>
#include <vector>

class FallProbInfer {
public:
	// ctor 傳入 TorchScript 模型路徑
	explicit FallProbInfer(const std::string& ts_model_path);

	// 1 筆資料：9 維 -> 機率百分比 [0,100]
	float inferOne(const std::array<float, 9>& features);

	// 多筆 batch：N x 9 -> N 個百分比
	std::vector<float> inferBatch(const std::vector<std::array<float, 9>>& batch);

private:
	torch::jit::script::Module module_;
	torch::Device device_ = torch::kCPU;

	torch::Tensor toTensor(const std::vector<std::array<float, 9>>& batch);
	static torch::Device selectDevice();
};