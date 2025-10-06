#include "scan_handler.hpp"

#include "util/logging.hpp"

#include <nlohmann/json.hpp>

#include <future>
#include <utility>

using boost::asio::awaitable;

ScanCommandHandler::ScanCommandHandler(
    ipcscan::ScanExecutor& executor, CommandPublishFn publish_response)
    : executor_(executor), publish_response_(std::move(publish_response)) {}

awaitable<void> ScanCommandHandler::Handle(const CommandMessage& command) {
  std::string response;
  try {
    auto future = std::async(std::launch::async, [&]() {
      return executor_.RunScan();
    });
    std::string result = future.get();
    response = BuildScanSuccess(command.trace_id, result);
    LogInfoFormat(
        "IPCscan 完成，trace_id={} 結果長度 {}",
        command.trace_id,
        result.size());
    LogInfoFormat(
        "IPCscan 結果內容 trace_id={} result={}", command.trace_id, result);
  } catch (const std::exception& ex) {
    LogErrorFormat("IPCscan 執行失敗: {}", ex.what());
    response = BuildScanError(command.trace_id, ex.what());
  } catch (...) {
    LogError("IPCscan 執行發生未知錯誤");
    response = BuildScanError(command.trace_id, "unknown error");
  }

  const bool published =
      co_await publish_response_(response, "發佈 IPCscan 結果失敗");
  if (published) {
    LogInfo("IPCscan 結果已送出");
  }
}

std::string ScanCommandHandler::BuildScanSuccess(
    const std::string& trace_id, const std::string& result_json) {
  nlohmann::json message{
      {"trace_id", trace_id}, {"code", 101}, {"status", "ok"}};
  nlohmann::json data = nlohmann::json::parse(result_json, nullptr, false);
  if (data.is_discarded()) {
    data = nlohmann::json::array();
  }
  message["result"] = std::move(data);
  return message.dump();
}

std::string ScanCommandHandler::BuildScanError(
    const std::string& trace_id, std::string_view error_message) {
  nlohmann::json message{
      {"trace_id", trace_id},
      {"code", 101},
      {"status", "error"},
      {"result", nlohmann::json{{"error_message", error_message}}}};
  return message.dump();
}
