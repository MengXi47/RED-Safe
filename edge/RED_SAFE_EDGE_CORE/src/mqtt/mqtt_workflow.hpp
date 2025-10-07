#pragma once

#include "../core/env_setting.hpp"
#include "dependency_injection.hpp"
#include "handler/command_handler.hpp"
#include "handler/heartbeat_handler.hpp"
#include "handler/network_info_handler.hpp"
#include "handler/scan_handler.hpp"
#include "handler/unsupported_command_handler.hpp"
#include "http/http_client.hpp"
#include "mqtt/mqtt_traits.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
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

  MqttWorkflow(
      boost::asio::io_context& io_context,
      mqtt_client& client,
      EdgeConfig config,
      IEdgeOnlineService& online_service);

  boost::asio::awaitable<void> Run();

 private:
  boost::asio::awaitable<bool> SubscribeCommands();
  boost::asio::awaitable<void> ConsumeCommands();
  void InitializeHandlers();
  void RequestReconnect(const boost::system::error_code& ec);
  [[nodiscard]] boost::asio::awaitable<bool> PublishCommandResponse(
      std::string payload, std::string_view error_context);
  [[nodiscard]] boost::asio::awaitable<bool> PublishStatusMessage(
      std::string payload, std::string_view error_context);
  void ResetCommandKeepalive();
  void HandleCommandTimeout(const boost::system::error_code& ec);

  boost::asio::io_context& io_context_;
  mqtt_client& client_;
  EdgeConfig config_;
  IEdgeOnlineService& online_service_;
  std::string status_topic_;
  std::string cmd_topic_;
  std::string data_topic_;
  boost::asio::steady_timer command_keepalive_timer_;
  static constexpr std::chrono::seconds command_keepalive_timeout_{60};
  std::unordered_map<std::string, std::unique_ptr<ICommandHandler>> handlers_;
  std::unique_ptr<UnsupportedCommandHandler> unsupported_handler_holder_;
  UnsupportedCommandHandler* unsupported_handler_{nullptr};
  HeartbeatHandler* heartbeat_handler_{nullptr};
  bool heartbeat_started_{false};
  bool reconnect_requested_{false};
  std::optional<boost::system::error_code> last_error_;
  static constexpr std::chrono::seconds reconnect_delay_{5};
  bool connected_{false};
};
