#include "unsupported_command_handler.hpp"

#include "util/logging.hpp"

#include <nlohmann/json.hpp>

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
  nlohmann::json message{
      {"trace_id", trace_id},
      {"code", code},
      {"status", "error"},
      {"result", nlohmann::json{{"error_message", "unsupported command"}}}};
  return message.dump();
}
