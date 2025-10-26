#include "mqtt/handler/heartbeat_handler.hpp"

#include "grpc/ip_resolver.hpp"
#include "util/logging.hpp"
#include "util/time.hpp"

#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <folly/dynamic.h>
#include <folly/json.h>

using boost::asio::awaitable;

HeartbeatHandler::HeartbeatHandler(
    EdgeConfig& config,
    CommandPublishFn publish_status,
    CommandPublishFn publish_response,
    std::function<void()> reset_keepalive)
    : config_(config),
      publish_status_(std::move(publish_status)),
      publish_response_(std::move(publish_response)),
      reset_keepalive_(std::move(reset_keepalive)) {}

// 處理心跳確認指令，重置 watchdog 並回傳 ACK
awaitable<void> HeartbeatHandler::Handle(const CommandMessage& command) {
  // 每收到一次心跳確認就重置 keepalive
  reset_keepalive_();
  const std::string response = BuildAckMessage(command.trace_id, command.code);
  co_await publish_response_(response, "發佈心跳確認失敗");
}

// 以協程方式定期發送心跳訊息
awaitable<void> HeartbeatHandler::RunPublisher() {
  boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
  std::uint64_t sequence = 0;

  for (;;) {
    RefreshEdgeIp();
    const std::string payload = BuildHeartbeatPayload(sequence++);
    co_await publish_status_(payload, "MQTT 心跳發佈失敗");

    timer.expires_after(config_.heartbeat_interval);
    boost::system::error_code wait_ec;
    co_await timer.async_wait(
        boost::asio::redirect_error(boost::asio::use_awaitable, wait_ec));
    if (wait_ec) {
      break;
    }
  }
  co_return;
}

std::string HeartbeatHandler::BuildHeartbeatPayload(
    std::uint64_t sequence) const {
  folly::dynamic payload = folly::dynamic::object;
  payload["edge_id"] = config_.edge_id;
  payload["version"] = config_.version;
  payload["heartbeat_at"] = CurrentIsoTimestamp();
  payload["status"] = "online";
  payload["sequence"] = sequence;
  payload["ip"] = config_.edge_ip;
  return folly::toJson(payload);
}

std::string HeartbeatHandler::BuildAckMessage(
    const std::string& trace_id, const std::string& code) {
  folly::dynamic message = folly::dynamic::object;
  message["trace_id"] = trace_id;
  message["code"] = code;
  message["status"] = "ok";
  message["result"] = folly::dynamic::object("message", "heartbeat_ack");
  return folly::toJson(message);
}

void HeartbeatHandler::RefreshEdgeIp() {
#ifndef __APPLE__
  const auto now = std::chrono::steady_clock::now();
  const bool missing_ip = config_.edge_ip.empty();
  const bool interval_elapsed =
      !has_refreshed_ip_ || (now - last_ip_refresh_at_) >= ip_refresh_interval_;

  if (!missing_ip && !interval_elapsed) {
    return;
  }

  if (auto ip = FetchEdgeIpFromIptool(config_)) {
    config_.edge_ip = *ip;
    last_ip_refresh_at_ = now;
    has_refreshed_ip_ = true;
    // LogInfoFormat("取得 Edge IP: {}", config_.edge_ip);
  } else if (missing_ip) {
    LogWarn("取得 Edge IP 失敗");
  }
#endif
}
