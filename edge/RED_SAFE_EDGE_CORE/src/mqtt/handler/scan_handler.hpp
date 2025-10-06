#pragma once

#include "command_handler.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/thread_pool.hpp>

#include <chrono>
#include <exception>
#include <functional>
#include <string_view>
#include <tuple>

// IPC 掃描指令處理器 code=101
class ScanCommandHandler final : public ICommandHandler {
 public:
  ScanCommandHandler(
      std::chrono::milliseconds scan_timeout,
      CommandPublishFn publish_response);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

 private:
  static std::string BuildScanSuccess(
      const std::string& trace_id, const std::string& result_json);
  static std::string BuildScanError(
      const std::string& trace_id, std::string_view error_message);
  boost::asio::awaitable<std::tuple<bool, std::string>>
  RunScanAsync();

  CommandPublishFn publish_response_;
  std::chrono::milliseconds scan_timeout_;
  boost::asio::thread_pool scan_pool_;
};
