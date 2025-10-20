#pragma once

#include "mqtt/handler/command_handler.hpp"

#include "core/env_setting.hpp"

#include <boost/asio/awaitable.hpp>

// 網路資訊查詢處理器 code=102
class NetworkInfoHandler : public ICommandHandler {
 public:
  explicit NetworkInfoHandler(
      EdgeConfig& config, CommandPublishFn publish_response);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

 private:
  CommandPublishFn publish_response_;
  EdgeConfig& config_;
};
