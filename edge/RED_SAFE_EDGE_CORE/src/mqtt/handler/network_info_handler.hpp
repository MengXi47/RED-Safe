#pragma once

#include "command_handler.hpp"

#include "../../core/env_setting.hpp"

#include <boost/asio/awaitable.hpp>

// 網路資訊查詢處理器 code=102
class NetworkInfoHandler : public ICommandHandler {
 public:
  explicit NetworkInfoHandler(EdgeConfig& config);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

 private:
  EdgeConfig& config_;
};
