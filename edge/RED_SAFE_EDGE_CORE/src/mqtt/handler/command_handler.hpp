#pragma once

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string>
#include <string_view>

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
};

// 統一的回覆函式型別：handler 可用來發佈 JSON 字串並取得成功與否
using CommandPublishFn = std::function<boost::asio::awaitable<bool>(
    std::string payload, std::string_view error_context)>;
