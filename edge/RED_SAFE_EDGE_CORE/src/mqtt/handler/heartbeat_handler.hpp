#pragma once

#include "mqtt/handler/command_handler.hpp"

#include "core/env_setting.hpp"

#include <boost/asio/awaitable.hpp>

#include <chrono>
#include <functional>

// 心跳處理器，負責回應 code=100 並定期推送狀態
class HeartbeatHandler final : public ICommandHandler {
 public:
  HeartbeatHandler(
      EdgeConfig& config,
      CommandPublishFn publish_status,
      CommandPublishFn publish_response,
      std::function<void()> reset_keepalive);

  boost::asio::awaitable<void> Handle(const CommandMessage& command) override;

  [[nodiscard]] boost::asio::awaitable<void> RunPublisher();

 private:
  [[nodiscard]] std::string BuildHeartbeatPayload(std::uint64_t sequence) const;
  [[nodiscard]] static std::string BuildAckMessage(
      const std::string& trace_id, const std::string& code);
  void RefreshEdgeIp();

  EdgeConfig& config_;
  CommandPublishFn publish_status_;
  CommandPublishFn publish_response_;
  std::function<void()> reset_keepalive_;
  std::chrono::steady_clock::time_point last_ip_refresh_at_{};
  bool has_refreshed_ip_{false};
  static constexpr std::chrono::minutes ip_refresh_interval_{10};
};
