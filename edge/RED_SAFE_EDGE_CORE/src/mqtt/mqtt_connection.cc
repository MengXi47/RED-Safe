#include "mqtt/mqtt_workflow.hpp"

#include "util/logging.hpp"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/mqtt5/reason_codes.hpp>
#include <boost/system/system_error.hpp>

#include <memory>
#include <utility>

using boost::asio::awaitable;
namespace mqtt5 = boost::mqtt5;

MqttWorkflow::MqttWorkflow(
    boost::asio::io_context& io_context,
    mqtt_client& client,
    EdgeConfig config,
    IEdgeOnlineService& online_service)
    : io_context_(io_context),
      client_(client),
      config_(std::move(config)),
      online_service_(online_service),
      status_topic_(config_.edge_id + "/status"),
      cmd_topic_(config_.edge_id + "/cmd"),
      data_topic_(config_.edge_id + "/data"),
      command_keepalive_timer_(io_context_) {
  InitializeHandlers();
}

awaitable<void> MqttWorkflow::Run() {
  // 啟動 MQTT client 並保持背景執行
  client_.brokers(config_.mqtt_broker, config_.mqtt_port)
      .credentials(
          config_.mqtt_client_id, config_.mqtt_username, config_.mqtt_password)
      .keep_alive(30)
      .async_run(boost::asio::detached);

  LogInfoFormat(
      "已啟動 MQTT 連線，broker={} port={}",
      config_.mqtt_broker,
      config_.mqtt_port);

  ResetCommandKeepalive();

  if (!(co_await SubscribeCommands())) {
    LogError("初始化訂閱指令 Topic 失敗，停止 MQTT 工作流程");
    co_return;
  }

  const auto executor = co_await boost::asio::this_coro::executor;
  if (heartbeat_handler_ != nullptr) {
    // 心跳 handler 另開協程定期發送狀態
    boost::asio::co_spawn(
        executor, heartbeat_handler_->RunPublisher(), boost::asio::detached);
  }

  co_await ConsumeCommands();
}

awaitable<bool> MqttWorkflow::SubscribeCommands() {
  // 訂閱 edge 專屬 /cmd topic
  const auto topic = mqtt5::subscribe_topic{
      cmd_topic_,
      mqtt5::subscribe_options{
          mqtt5::qos_e::at_least_once,
          mqtt5::no_local_e::no,
          mqtt5::retain_as_published_e::retain,
          mqtt5::retain_handling_e::send}};

  auto [ec, codes, props] = co_await client_.async_subscribe(
      topic,
      mqtt5::subscribe_props{},
      boost::asio::as_tuple(boost::asio::deferred));
  (void)props;

  if (ec) {
    LogErrorFormat("訂閱 MQTT Topic {} 失敗: {}", cmd_topic_, ec.message());
    co_return false;
  }
  if (!codes.empty() && !!codes.front()) {
    LogErrorFormat(
        "MQTT Broker 拒絕訂閱 {}: {}", cmd_topic_, codes.front().message());
    co_return false;
  }

  LogInfoFormat("已訂閱 MQTT Topic {}", cmd_topic_);
  co_return true;
}

void MqttWorkflow::ResetCommandKeepalive() {
  try {
    command_keepalive_timer_.cancel();
  } catch (const boost::system::system_error&) {
    // Ignore cancellation errors when timer not pending.
  }
  // 重設 watchdog，若超時會觸發重新上線邏輯
  command_keepalive_timer_.expires_after(command_keepalive_timeout_);
  command_keepalive_timer_.async_wait(
      [this](const boost::system::error_code& error) {
        HandleCommandTimeout(error);
      });
}

void MqttWorkflow::HandleCommandTimeout(const boost::system::error_code& ec) {
  if (ec == boost::asio::error::operation_aborted) {
    return;
  }

  LogWarn("指令心跳超時 60 秒，嘗試重新送出上線請求");
  if (online_service_.ReportOnline(config_)) {
    LogInfo("重新送出上線請求成功");
  } else {
    LogWarn("重新送出上線請求失敗");
    LogError("重新上線失敗，準備終止程式");
    std::terminate();
  }
  ResetCommandKeepalive();
}

// 建立各類指令處理器並掛入分派表
void MqttWorkflow::InitializeHandlers() {
  const CommandPublishFn publish_response =
      [this](std::string payload, std::string_view error_context)
      -> awaitable<bool> {
    co_return co_await PublishCommandResponse(
        std::move(payload), error_context);
  };

  const CommandPublishFn publish_status =
      [this](std::string payload, std::string_view error_context)
      -> awaitable<bool> {
    co_return co_await PublishStatusMessage(std::move(payload), error_context);
  };

  std::function reset_keepalive = [this] { ResetCommandKeepalive(); };

  // 交由 dependency_injection.hpp 集中註冊所有內建 handler
  RegisterDefaultHandlers(
      handlers_,
      heartbeat_handler_,
      unsupported_handler_holder_,
      unsupported_handler_,
      config_,
      publish_status,
      publish_response,
      std::move(reset_keepalive));
}

// 發佈訊息到 data topic
awaitable<bool> MqttWorkflow::PublishCommandResponse(
    std::string payload, std::string_view error_context) const {
  // 由 handler 呼叫，發佈指令執行結果
  auto [publish_ec, command_code, command_props] =
      co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
          data_topic_,
          std::move(payload),
          mqtt5::retain_e::no,
          mqtt5::publish_props{},
          boost::asio::as_tuple(boost::asio::deferred));
  (void)command_code;
  (void)command_props;
  if (publish_ec) {
    LogErrorFormat("{}: {}", error_context, publish_ec.message());
    co_return false;
  }
  co_return true;
}

// 發佈訊息到 status topic
awaitable<bool> MqttWorkflow::PublishStatusMessage(
    std::string payload, std::string_view error_context) const {
  auto [publish_ec, puback_code, puback_props] =
      co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
          status_topic_,
          std::move(payload),
          mqtt5::retain_e::no,
          mqtt5::publish_props{},
          boost::asio::as_tuple(boost::asio::deferred));
  (void)puback_code;
  (void)puback_props;
  if (publish_ec) {
    LogWarnFormat("{}: {}", error_context, publish_ec.message());
    co_return false;
  }
  co_return true;
}
