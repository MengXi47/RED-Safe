#pragma once

#include "core/config.hpp"
#include "core/http_client.hpp"
#include "core/mqtt_traits.hpp"
#include "ipcscan/app/scan_executor.hpp"

#include <chrono>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/mqtt5/logger.hpp>
#include <boost/mqtt5/mqtt_client.hpp>
#include <boost/mqtt5/websocket_ssl.hpp>

class MqttWorkflow {
 public:
  using mqtt_client = boost::mqtt5::mqtt_client<
      boost::beast::websocket::stream<
          boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>,
      boost::asio::ssl::context,
      boost::mqtt5::logger>;

  // 建構 MQTT 工作流程，注入核心依賴與保持存活所需的服務
  MqttWorkflow(
      boost::asio::io_context& io_context,
      mqtt_client& client,
      EdgeConfig config,
      ipcscan::ScanExecutor& executor,
      IEdgeOnlineService& online_service);

  boost::asio::awaitable<void> Run();

 private:
  // 訂閱 Edge 指令 Topic
  boost::asio::awaitable<bool> SubscribeCommands();
  // 定時發佈 Edge 心跳狀態
  boost::asio::awaitable<void> PublishHeartbeat();
  // 持續處理指令 Topic 的訊息
  boost::asio::awaitable<void> ConsumeCommands();
  // 重新啟動指令心跳 watchdog
  void ResetCommandKeepalive();
  // 指令心跳逾時的處理流程
  void HandleCommandTimeout(const boost::system::error_code& ec);

  // 建構心跳發佈的 JSON payload
  std::string BuildHeartbeatPayload(std::uint64_t sequence) const;
  // 建構 IPCscan 成功回傳
  std::string BuildScanSuccess(
      const std::string& trace_id, const std::string& result_json) const;
  // 建構 IPCscan 失敗回傳
  std::string BuildScanError(
      const std::string& trace_id, std::string_view error_message) const;
  // 建構指令成功 ACK
  std::string BuildAckMessage(
      const std::string& trace_id, const std::string& code) const;
  // 建構未支援指令的錯誤回應
  std::string BuildUnsupportedCommand(
      const std::string& trace_id, const std::string& code) const;

  boost::asio::io_context& io_context_;
  mqtt_client& client_;
  EdgeConfig config_;
  ipcscan::ScanExecutor& executor_;
  IEdgeOnlineService& online_service_;
  std::string status_topic_;
  std::string cmd_topic_;
  std::string data_topic_;
  boost::asio::steady_timer command_keepalive_timer_;
  static constexpr std::chrono::seconds command_keepalive_timeout_{60};
  static constexpr auto use_nothrow_awaitable =
      boost::asio::as_tuple(boost::asio::deferred);
};
