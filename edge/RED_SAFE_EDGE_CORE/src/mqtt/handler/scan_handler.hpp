#pragma once

#include "command_handler.hpp"

#include "../../ipcscan/app/scan_executor.hpp"

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string_view>

// IPC 掃描指令處理器 code=101
class ScanCommandHandler final : public ICommandHandler {
 public:
  ScanCommandHandler(
      ipcscan::ScanExecutor& executor, CommandPublishFn publish_response);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

 private:
  static std::string BuildScanSuccess(
      const std::string& trace_id, const std::string& result_json);
  static std::string BuildScanError(
      const std::string& trace_id, std::string_view error_message);

  ipcscan::ScanExecutor& executor_;
  CommandPublishFn publish_response_;
};
