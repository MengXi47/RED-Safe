// MQTT 指令流程與 Edge 端整合邏輯
#include "mqtt/mqtt_workflow.hpp"

#include "common/logging.hpp"
#include "common/time.hpp"
#include "mqtt/mqtt_traits.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/mqtt5/reason_codes.hpp>
#include <boost/system/system_error.hpp>

#include <nlohmann/json.hpp>

#include <future>
#include <string_view>

#include "grpc/grpc_client.hpp"

using boost::asio::awaitable;
namespace mqtt5 = boost::mqtt5;

// 建構函式：初始化 MQTT 工作流程與保持存活機制
MqttWorkflow::MqttWorkflow(
    boost::asio::io_context& io_context,
    mqtt_client& client,
    EdgeConfig config,
    ipcscan::ScanExecutor& executor,
    IEdgeOnlineService& online_service)
    : io_context_(io_context),
      client_(client),
      config_(std::move(config)),
      executor_(executor),
      online_service_(online_service),
      status_topic_(config_.edge_id + "/status"),
      cmd_topic_(config_.edge_id + "/cmd"),
      data_topic_(config_.edge_id + "/data"),
      command_keepalive_timer_(io_context_) {}

// 啟動整體工作流程：連線、訂閱、心跳與指令循環
awaitable<void> MqttWorkflow::Run() {
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
  boost::asio::co_spawn(executor, PublishHeartbeat(), boost::asio::detached);

  co_await ConsumeCommands();
}

// 訂閱 Edge 指令 Topic，確保只註冊一次
awaitable<bool> MqttWorkflow::SubscribeCommands() {
  mqtt5::subscribe_topic topic = mqtt5::subscribe_topic{
      cmd_topic_,
      mqtt5::subscribe_options{
          mqtt5::qos_e::at_least_once,
          mqtt5::no_local_e::no,
          mqtt5::retain_as_published_e::retain,
          mqtt5::retain_handling_e::send}};

  auto [ec, codes, props] = co_await client_.async_subscribe(
      topic, mqtt5::subscribe_props{}, use_nothrow_awaitable);
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

// 透過 MQTT 定期發出 Edge 心跳訊息
awaitable<void> MqttWorkflow::PublishHeartbeat() const {
  boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
  std::uint64_t sequence = 0;

  for (;;) {
    const std::string payload = BuildHeartbeatPayload(sequence++);
    auto [publish_ec, puback_code, puback_props] =
        co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
            status_topic_,
            payload,
            mqtt5::retain_e::no,
            mqtt5::publish_props{},
            use_nothrow_awaitable);
    (void)puback_code;
    (void)puback_props;
    if (publish_ec) {
      LogWarnFormat("MQTT 心跳發佈失敗: {}", publish_ec.message());
    } else {
      LogDebugFormat("心跳已送出: {}", payload);
    }

    timer.expires_after(config_.heartbeat_interval);
    auto [wait_ec] = co_await timer.async_wait(use_nothrow_awaitable);
    if (wait_ec) {
      break;
    }
  }
  co_return;
}

// 處理來自 /cmd Topic 的所有指令
awaitable<void> MqttWorkflow::ConsumeCommands() {
  for (;;) {
    auto [ec, topic, payload, props] =
        co_await client_.async_receive(use_nothrow_awaitable);
    (void)props;

    if (ec == mqtt5::client::error::session_expired) {
      LogWarn("MQTT 會話過期，嘗試重新訂閱");
      if (!(co_await SubscribeCommands())) {
        LogError("重新訂閱指令 Topic 失敗，停止 command loop");
        co_return;
      }
      continue;
    }
    if (ec) {
      if (ec != boost::asio::error::operation_aborted) {
        LogErrorFormat("接收 MQTT 訊息失敗: {}", ec.message());
      }
      co_return;
    }

    if (topic != cmd_topic_) {
      LogWarnFormat("收到未知 Topic {} 的訊息，忽略", topic);
      continue;
    }

    nlohmann::json json = nlohmann::json::parse(payload, nullptr, false);
    if (json.is_discarded()) {
      LogWarnFormat("解析 MQTT 指令失敗，payload={}", payload);
      continue;
    }

    const auto trace_it = json.find("trace_id");
    const auto code_it = json.find("code");
    std::string trace_value = (trace_it != json.end() && trace_it->is_string())
        ? trace_it->get<std::string>()
        : "";
    std::string code_field_value;
    if (code_it != json.end()) {
      if (code_it->is_string()) {
        code_field_value = code_it->get<std::string>();
      } else if (code_it->is_number_integer()) {
        code_field_value = std::to_string(code_it->get<int>());
      } else {
        code_field_value = code_it->dump();
      }
    }

    if (trace_it == json.end() || !trace_it->is_string() ||
        code_it == json.end()) {
      LogWarnFormat("MQTT 指令欄位不完整，payload={}", payload);
      std::string response =
          BuildUnsupportedCommand(trace_value, code_field_value);
      auto [publish_ec, command_code, command_props] =
          co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
              data_topic_,
              response,
              mqtt5::retain_e::no,
              mqtt5::publish_props{},
              use_nothrow_awaitable);
      (void)command_code;
      (void)command_props;
      if (publish_ec) {
        LogErrorFormat("發佈指令錯誤報告失敗: {}", publish_ec.message());
      }
      continue;
    }

    const std::string trace_id = *trace_it;
    std::string code_str;
    if (code_it->is_string()) {
      code_str = *code_it;
    } else if (code_it->is_number_integer()) {
      code_str = std::to_string(code_it->get<int>());
    } else {
      LogWarnFormat("MQTT 指令 code 欄位型別不支援，payload={}", payload);
      std::string response =
          BuildUnsupportedCommand(trace_id, code_field_value);
      auto [publish_ec, command_code, command_props] =
          co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
              data_topic_,
              response,
              mqtt5::retain_e::no,
              mqtt5::publish_props{},
              use_nothrow_awaitable);
      (void)command_code;
      (void)command_props;
      if (publish_ec) {
        LogErrorFormat("發佈指令錯誤報告失敗: {}", publish_ec.message());
      }
      continue;
    }

    LogInfoFormat("收到指令 code={} trace_id={}", code_str, trace_id);

    if (code_str == "100") {
      ResetCommandKeepalive();
      std::string response = BuildAckMessage(trace_id, code_str);
      auto [publish_ec, command_code, command_props] =
          co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
              data_topic_,
              response,
              mqtt5::retain_e::no,
              mqtt5::publish_props{},
              use_nothrow_awaitable);
      (void)command_code;
      (void)command_props;
      if (publish_ec) {
        LogErrorFormat("發佈心跳確認失敗: {}", publish_ec.message());
      }
      continue;
    }

    if (code_str == "101") {
      std::string response;
      try {
        std::string result;
        auto future = std::async(std::launch::async, [&]() {
          return executor_.RunScan();
        });
        result = future.get();
        response = BuildScanSuccess(trace_id, result);
        LogInfoFormat(
            "IPCscan 完成，trace_id={} 結果長度 {}", trace_id, result.size());
        LogInfoFormat(
            "IPCscan 結果內容 trace_id={} result={}", trace_id, result);
      } catch (const std::exception& ex) {
        LogErrorFormat("IPCscan 執行失敗: {}", ex.what());
        response = BuildScanError(trace_id, ex.what());
      } catch (...) {
        LogError("IPCscan 執行發生未知錯誤");
        response = BuildScanError(trace_id, "unknown error");
      }

      auto [publish_ec, command_code, command_props] =
          co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
              data_topic_,
              response,
              mqtt5::retain_e::no,
              mqtt5::publish_props{},
              use_nothrow_awaitable);
      (void)command_code;
      (void)command_props;
      if (publish_ec) {
        LogErrorFormat("發佈 IPCscan 結果失敗: {}", publish_ec.message());
      } else {
        LogInfo("IPCscan 結果已送出");
      }
      continue;
    }

    if (code_str == "102") {
      std::string target_str = "localhost:20002"; // gRPC server 位址
      std::string interface_name = "en9"; // 預設查詢介面
      NetworkServiceClient client(
          grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

      client.GetNetworkConfig(interface_name);
      continue;
    }

    LogWarnFormat(
        "收到未支援的指令 code={} trace_id={}，回應錯誤", code_str, trace_id);
    std::string response = BuildUnsupportedCommand(trace_id, code_str);
    auto [publish_ec, command_code, command_props] =
        co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
            data_topic_,
            response,
            mqtt5::retain_e::no,
            mqtt5::publish_props{},
            use_nothrow_awaitable);
    (void)command_code;
    (void)command_props;
    if (publish_ec) {
      LogErrorFormat("發佈未支援指令錯誤報告失敗: {}", publish_ec.message());
    }
  }
}

// 重置指令心跳 watchdog，逾時將觸發重新上線
void MqttWorkflow::ResetCommandKeepalive() {
  try {
    command_keepalive_timer_.cancel();
  } catch (const boost::system::system_error&) {
    // Ignore cancellation errors when timer not pending.
  }
  command_keepalive_timer_.expires_after(command_keepalive_timeout_);
  command_keepalive_timer_.async_wait(
      [this](const boost::system::error_code& error) {
        HandleCommandTimeout(error);
      });
}

// 指令逾時觸發的重新上線程序
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

// 心跳 payload 組裝
std::string MqttWorkflow::BuildHeartbeatPayload(std::uint64_t sequence) const {
  nlohmann::json payload{
      {"edge_id", config_.edge_id},
      {"version", config_.version},
      {"heartbeat_at", CurrentIsoTimestamp()},
      {"status", "online"},
      {"sequence", sequence}};
  return payload.dump();
}

// IPCscan 成功回報 JSON（包含 trace_id 與 result 資料）
std::string MqttWorkflow::BuildScanSuccess(
    const std::string& trace_id, const std::string& result_json) {
  nlohmann::json message{
      {"trace_id", trace_id}, {"code", 101}, {"status", "ok"}};
  nlohmann::json data = nlohmann::json::parse(result_json, nullptr, false);
  if (data.is_discarded()) {
    data = nlohmann::json::array();
  }
  message["result"] = std::move(data);
  return message.dump();
}

// IPCscan 失敗回報 JSON（包含錯誤 detail）
std::string MqttWorkflow::BuildScanError(
    const std::string& trace_id, std::string_view error_message) {
  nlohmann::json message{
      {"trace_id", trace_id},
      {"code", 101},
      {"status", "error"},
      {"result", nlohmann::json{{"error_message", error_message}}}};
  return message.dump();
}

// 建構心跳確認的成功回應
std::string MqttWorkflow::BuildAckMessage(
    const std::string& trace_id, const std::string& code) {
  nlohmann::json message{
      {"trace_id", trace_id},
      {"code", code},
      {"status", "ok"},
      {"result", nlohmann::json{{"message", "heartbeat_ack"}}}};
  return message.dump();
}

// 建構未支援指令的錯誤回應（保留原始 code）
std::string MqttWorkflow::BuildUnsupportedCommand(
    const std::string& trace_id, const std::string& code) {
  nlohmann::json message{
      {"trace_id", trace_id},
      {"code", code},
      {"status", "error"},
      {"result", nlohmann::json{{"error_message", "unsupported command"}}}};
  return message.dump();
}
