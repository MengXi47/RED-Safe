#include "scan_handler.hpp"

#include "../../ipcscan/app/scan_executor.hpp"
#include "util/logging.hpp"

#include <boost/asio.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <thread>
#include <utility>

using boost::asio::awaitable;

ScanCommandHandler::ScanCommandHandler(
    std::chrono::milliseconds scan_timeout, CommandPublishFn publish_response)
    : publish_response_(std::move(publish_response)),
      scan_timeout_(scan_timeout),
      scan_pool_(std::max(1u, std::thread::hardware_concurrency())) {}

awaitable<void> ScanCommandHandler::Handle(const CommandMessage& command) {
  std::string response;
  auto [success, result] = co_await RunScanAsync();

  if (!success) {
    LogErrorFormat("IPCscan 執行失敗: {}", result);
    response = BuildScanError(command.trace_id, result);
  } else {
    LogInfoFormat(
        "IPCscan 完成，trace_id={} 結果長度 {}",
        command.trace_id,
        result.size());
    LogInfoFormat(
        "IPCscan 結果內容 trace_id={} result={}", command.trace_id, result);
    response = BuildScanSuccess(command.trace_id, result);
  }

  const bool published =
      co_await publish_response_(response, "發佈 IPCscan 結果失敗");
  if (published) {
    LogInfo("IPCscan 結果已送出");
  }
}

awaitable<std::tuple<bool, std::string>> ScanCommandHandler::RunScanAsync() {
  auto executor = co_await boost::asio::this_coro::executor;
  co_return co_await boost::asio::async_initiate<
      decltype(boost::asio::use_awaitable),
      void(bool, std::string)>(
      [this, executor]<typename T0>(T0&& completion_handler) {
        using HandlerType = std::decay_t<T0>;
        auto handler =
            std::make_shared<HandlerType>(std::forward<T0>(completion_handler));

        boost::asio::post(
            scan_pool_, [handler, executor, timeout = scan_timeout_]() mutable {
              bool success = true;
              std::string result;
              try {
                ipcscan::ScanExecutor executor_obj(timeout);
                result = executor_obj.RunScan();
              } catch (...) {
                success = false;
                try {
                  throw;
                } catch (const std::exception& ex) {
                  result = ex.what();
                } catch (...) {
                  result = "unknown error";
                }
              }

              boost::asio::post(
                  executor,
                  [handler, success, result = std::move(result)]() mutable {
                    (*handler)(success, std::move(result));
                  });
            });
      },
      boost::asio::use_awaitable);
}

std::string ScanCommandHandler::BuildScanSuccess(
    const std::string& trace_id, const std::string& result_json) {
  nlohmann::json data = nlohmann::json::parse(result_json, nullptr, false);
  if (data.is_discarded()) {
    data = nlohmann::json::array();
  }
  return BuildSuccessResponse(trace_id, 101, std::move(data));
}

std::string ScanCommandHandler::BuildScanError(
    const std::string& trace_id, std::string_view error_message) {
  return BuildErrorResponse(trace_id, 101, error_message);
}
