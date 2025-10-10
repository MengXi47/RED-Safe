#pragma once

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include <nlohmann/json.hpp>

struct CommandMessage {
  std::string trace_id;
  std::string code;
  const nlohmann::json& payload;
  std::string raw_payload;
};

class ICommandHandler {
 public:
  virtual ~ICommandHandler() = default;

  virtual boost::asio::awaitable<void> Handle(
      const CommandMessage& command) = 0;

 protected:
  static std::string BuildSuccessResponse(
      std::string trace_id, int code, nlohmann::json result) {
    nlohmann::json message{
        {"trace_id", std::move(trace_id)},
        {"code", code},
        {"status", "ok"},
        {"result", std::move(result)}};
    return message.dump();
  }

  static std::string BuildErrorResponse(
      std::string trace_id,
      int code,
      std::string_view error_message) {
    nlohmann::json message{
        {"trace_id", std::move(trace_id)},
        {"code", code},
        {"status", "error"},
        {"result", nlohmann::json{{"error_message", error_message}}}};
    return message.dump();
  }
};

// 統一的回覆函式型別：handler 可用來發佈 JSON 字串並取得成功與否
using CommandPublishFn = std::function<boost::asio::awaitable<bool>(
    std::string payload, std::string_view error_context)>;
