#include "mqtt/mqtt_workflow.hpp"

#include "util/logging.hpp"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/error.hpp>

#include <nlohmann/json.hpp>

using boost::asio::awaitable;
namespace mqtt5 = boost::mqtt5;

namespace {
std::string ExtractCodeFieldValue(const nlohmann::json* code_field) {
  if (code_field == nullptr) {
    return "";
  }
  if (code_field->is_string()) {
    return code_field->get<std::string>();
  }
  if (code_field->is_number_integer()) {
    return std::to_string(code_field->get<int>());
  }
  return code_field->dump();
}
} // namespace

// 接收 /cmd Topic 並分派至對應的處理器
awaitable<void> MqttWorkflow::ConsumeCommands() {
  for (;;) {
    auto [ec, topic, payload, props] = co_await client_.async_receive(
        boost::asio::as_tuple(boost::asio::deferred));
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

    const std::string trace_value =
        (trace_it != json.end() && trace_it->is_string()) ? *trace_it : "";
    const std::string code_field_value =
        (code_it != json.end()) ? ExtractCodeFieldValue(&(*code_it)) : "";

    if (trace_it == json.end() || !trace_it->is_string() ||
        code_it == json.end()) {
      LogWarnFormat("MQTT 指令欄位不完整，payload={}", payload);
      CommandMessage command{trace_value, code_field_value, json, payload};
      co_await unsupported_handler_->Handle(command);
      continue;
    }

    std::string code_str;
    if (code_it->is_string()) {
      code_str = *code_it;
    } else if (code_it->is_number_integer()) {
      code_str = std::to_string(code_it->get<int>());
    } else {
      LogWarnFormat("MQTT 指令 code 欄位型別不支援，payload={}", payload);
      CommandMessage command{trace_value, code_field_value, json, payload};
      co_await unsupported_handler_->Handle(command);
      continue;
    }

    if (code_str != "100") {
      LogInfoFormat("收到指令 code={} trace_id={}", code_str, trace_value);
    }

    // 根據指令代碼查找對應 handler
    auto handler_it = handlers_.find(code_str);
    ICommandHandler* handler =
        handler_it != handlers_.end() ? handler_it->second.get() : nullptr;

    if (handler == nullptr) {
      LogWarnFormat(
          "收到未支援的指令 code={} trace_id={}，回應錯誤",
          code_str,
          trace_value);
      CommandMessage command{trace_value, code_str, json, payload};
      co_await unsupported_handler_->Handle(command);
      continue;
    }

    CommandMessage command{trace_value, code_str, json, payload};
    co_await handler->Handle(command);
  }
}
