#pragma once

#include "command_handler.hpp"

class GetIPCInfoHandler final : public ICommandHandler {
 public:
  explicit GetIPCInfoHandler(const CommandPublishFn& publish_response);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

 private:
  CommandPublishFn publish_response_;
};

class SetIPCInfoHandler final : public ICommandHandler {
public:
  explicit SetIPCInfoHandler(const CommandPublishFn& publish_response);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

private:
  CommandPublishFn publish_response_;
};

class DelIPCInfoHandler final : public ICommandHandler {
public:
  explicit DelIPCInfoHandler(const CommandPublishFn& publish_response);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

private:
  CommandPublishFn publish_response_;
};
