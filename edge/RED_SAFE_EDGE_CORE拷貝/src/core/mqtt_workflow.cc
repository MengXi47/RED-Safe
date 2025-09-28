#include "core/mqtt_workflow.hpp"

#include "common/logging.hpp"
#include "common/time.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/mqtt5/reason_codes.hpp>

#include <nlohmann/json.hpp>

#include <future>
#include <string_view>

using boost::asio::awaitable;
namespace mqtt5 = boost::mqtt5;

MqttWorkflow::MqttWorkflow(mqtt_client& client, EdgeConfig config, ipcscan::ScanExecutor& executor)
    : client_(client),
      config_(std::move(config)),
      executor_(executor),
      status_topic_(config_.edge_id + "/status"),
      cmd_topic_(config_.edge_id + "/cmd"),
      data_topic_(config_.edge_id + "/data") {}

awaitable<void> MqttWorkflow::Run() {
    client_.brokers(config_.mqtt_broker, config_.mqtt_port)
        .credentials(config_.mqtt_client_id, config_.mqtt_username, config_.mqtt_password)
        .keep_alive(30)
        .async_run(boost::asio::detached);

    LogInfoFormat("已啟動 MQTT 連線，broker={} port={}", config_.mqtt_broker, config_.mqtt_port);

    if (!(co_await SubscribeCommands())) {
        LogError("初始化訂閱指令 Topic 失敗，停止 MQTT 工作流程");
        co_return;
    }

    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::co_spawn(executor, PublishHeartbeat(), boost::asio::detached);

    co_await ConsumeCommands();
}

awaitable<bool> MqttWorkflow::SubscribeCommands() {
    mqtt5::subscribe_topic topic = mqtt5::subscribe_topic{
        cmd_topic_,
        mqtt5::subscribe_options{
            mqtt5::qos_e::at_least_once,
            mqtt5::no_local_e::no,
            mqtt5::retain_as_published_e::retain,
            mqtt5::retain_handling_e::send
        }
    };

    auto [ec, codes, props] = co_await client_.async_subscribe(
        topic, mqtt5::subscribe_props{}, use_nothrow_awaitable
    );
    (void)props;

    if (ec) {
        LogErrorFormat("訂閱 MQTT Topic {} 失敗: {}", cmd_topic_, ec.message());
        co_return false;
    }
    if (!codes.empty() && !!codes.front()) {
        LogErrorFormat("MQTT Broker 拒絕訂閱 {}: {}", cmd_topic_, codes.front().message());
        co_return false;
    }

    LogInfoFormat("已訂閱 MQTT Topic {}", cmd_topic_);
    co_return true;
}

awaitable<void> MqttWorkflow::PublishHeartbeat() {
    boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
    std::uint64_t sequence = 0;

    for (;;) {
        const std::string payload = BuildHeartbeatPayload(sequence++);
        auto [publish_ec] = co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
            status_topic_, payload, mqtt5::retain_e::no, mqtt5::publish_props{}, use_nothrow_awaitable
        );
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

awaitable<void> MqttWorkflow::ConsumeCommands() {
    for (;;) {
        auto [ec, topic, payload, props] = co_await client_.async_receive(use_nothrow_awaitable);
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
        if (trace_it == json.end() || !trace_it->is_string() ||
            code_it == json.end() || !code_it->is_number_integer()) {
            LogWarnFormat("MQTT 指令欄位不完整，payload={}", payload);
            continue;
        }

        const std::string trace_id = *trace_it;
        const int code = *code_it;
        LogInfoFormat("收到指令 code={} trace_id={}", code, trace_id);

        if (code == 101) {
            std::string result;
            std::string response;
            try {
                auto future = std::async(std::launch::async, [&]() { return executor_.RunScan(); });
                result = future.get();
                response = BuildScanSuccess(trace_id, result);
                LogInfoFormat("IPCscan 完成，結果長度 {}", result.size());
            } catch (const std::exception& ex) {
                LogErrorFormat("IPCscan 執行失敗: {}", ex.what());
                response = BuildScanError(trace_id, ex.what());
            } catch (...) {
                LogError("IPCscan 執行發生未知錯誤");
                response = BuildScanError(trace_id, "unknown error");
            }

            auto [publish_ec] = co_await client_.async_publish<mqtt5::qos_e::at_least_once>(
                data_topic_, response, mqtt5::retain_e::no, mqtt5::publish_props{}, use_nothrow_awaitable
            );
            if (publish_ec) {
                LogErrorFormat("發佈 IPCscan 結果失敗: {}", publish_ec.message());
            } else {
                LogInfo("IPCscan 結果已送出");
            }
        } else {
            LogWarnFormat("收到未支援的指令 code={} trace_id={}，忽略", code, trace_id);
        }
    }
}

std::string MqttWorkflow::BuildHeartbeatPayload(std::uint64_t sequence) const {
    nlohmann::json payload{
        {"edge_id", config_.edge_id},
        {"version", config_.version},
        {"heartbeat_at", CurrentIsoTimestamp()},
        {"status", "online"},
        {"sequence", sequence}
    };
    return payload.dump();
}

std::string MqttWorkflow::BuildScanSuccess(const std::string& trace_id, const std::string& result_json) const {
    nlohmann::json message{
        {"trace_id", trace_id},
        {"code", 101},
        {"status", "ok"}
    };
    nlohmann::json data = nlohmann::json::parse(result_json, nullptr, false);
    if (data.is_discarded()) {
        data = nlohmann::json::array();
    }
    message["data"] = std::move(data);
    return message.dump();
}

std::string MqttWorkflow::BuildScanError(const std::string& trace_id, std::string_view error_message) const {
    nlohmann::json message{
        {"trace_id", trace_id},
        {"code", 101},
        {"status", "error"},
        {"error_message", error_message}
    };
    return message.dump();
}
