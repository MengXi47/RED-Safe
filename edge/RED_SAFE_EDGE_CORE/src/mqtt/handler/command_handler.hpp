#pragma once

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include <folly/json.h>

struct CommandMessage {
  std::string trace_id;
  std::string code;
  const folly::dynamic& payload;
  const folly::dynamic& raw_payload;
};

class ICommandHandler {
 public:
  virtual ~ICommandHandler() = default;

  virtual boost::asio::awaitable<void> Handle(
      const CommandMessage& command) = 0;

 protected:
  static std::string BuildSuccessResponse(
      std::string trace_id, std::string_view code, folly::dynamic result) {
    folly::dynamic message = folly::dynamic::object;
    message["trace_id"] = std::move(trace_id);
    message["code"] = code;
    message["status"] = "ok";
    message["result"] = std::move(result);
    return folly::toJson(message);
  }

  static std::string BuildErrorResponse(
      std::string trace_id,
      std::string_view code,
      const std::string_view error_message) {
    folly::dynamic message = folly::dynamic::object;
    message["trace_id"] = std::move(trace_id);
    message["code"] = code;
    message["status"] = "error";
    message["result"] =
        folly::dynamic::object("error_message", std::string(error_message));
    return folly::toJson(message);
  }
};

// 統一的回覆函式型別：handler 可用來發佈 JSON 字串並取得成功與否
using CommandPublishFn = std::function<boost::asio::awaitable<bool>(
    std::string payload, std::string_view error_context)>;
