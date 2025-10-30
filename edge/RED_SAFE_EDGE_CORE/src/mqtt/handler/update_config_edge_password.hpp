#pragma once

#include "command_handler.hpp"

class UpdateEdgePassword final : public ICommandHandler {
 public:
  explicit UpdateEdgePassword(
      const CommandPublishFn& publish_response, std::string_view edge_id);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

 private:
  CommandPublishFn publish_response_;
  std::string_view edge_id_;
};