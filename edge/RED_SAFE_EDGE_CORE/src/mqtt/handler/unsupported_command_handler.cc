#include "mqtt/handler/unsupported_command_handler.hpp"

#include "util/logging.hpp"

#include <folly/dynamic.h>
#include <folly/json.h>

#include <utility>

using boost::asio::awaitable;

UnsupportedCommandHandler::UnsupportedCommandHandler(
    CommandPublishFn publish_response)
    : publish_response_(std::move(publish_response)) {}

awaitable<void> UnsupportedCommandHandler::Handle(
    const CommandMessage& command) {
  const std::string response =
      BuildUnsupportedCommand(command.trace_id, command.code);
  // 發佈時帶上錯誤描述，方便遠端追蹤
  co_await publish_response_(response, "發佈未支援指令錯誤報告失敗");
}

std::string UnsupportedCommandHandler::BuildUnsupportedCommand(
    const std::string& trace_id, const std::string& code) {
  folly::dynamic message = folly::dynamic::object;
  message["trace_id"] = trace_id;
  message["code"] = code;
  message["status"] = "error";
  message["result"] =
      folly::dynamic::object("error_message", "unsupported command");
  return folly::toJson(message);
}
