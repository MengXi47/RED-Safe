#pragma once

#include "command_handler.hpp"

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string_view>

// 未支援指令處理器
class UnsupportedCommandHandler : public ICommandHandler {
 public:
  explicit UnsupportedCommandHandler(CommandPublishFn publish_response);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

 private:
  static std::string BuildUnsupportedCommand(
      const std::string& trace_id, const std::string& code) ;

  CommandPublishFn publish_response_;
};
