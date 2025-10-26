#include "mqtt/mqtt_workflow.hpp"

#include "util/logging.hpp"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/errc.hpp>

#include <folly/Conv.h>
#include <folly/dynamic.h>
#include <folly/json.h>

using boost::asio::awaitable;
namespace mqtt5 = boost::mqtt5;

namespace {
std::string ExtractCodeFieldValue(const folly::dynamic* code_field) {
  if (code_field == nullptr) {
    return "";
  }
  if (code_field->isString()) {
    return code_field->asString();
  }
  if (code_field->isInt()) {
    return folly::to<std::string>(code_field->asInt());
  }
  if (code_field->isDouble()) {
    return folly::to<std::string>(code_field->asDouble());
  }
  return folly::toJson(*code_field);
}
} // namespace

// 接收 /cmd Topic 並分派至對應的處理器
awaitable<void> MqttWorkflow::ConsumeCommands() {
  for (;;) {
    auto [ec, topic, raw_payload, props] = co_await client_.async_receive(
        boost::asio::as_tuple(boost::asio::deferred));
    (void)props;

    if (ec == mqtt5::client::error::session_expired) {
      LogWarn("MQTT 會話過期，嘗試重新訂閱");
      if (!(co_await SubscribeCommands())) {
        LogError("重新訂閱指令 Topic 失敗，準備重新連線");
        RequestReconnect(
            boost::system::errc::make_error_code(
                boost::system::errc::not_connected));
        co_return;
      }
      continue;
    }

    if (ec == boost::asio::error::operation_aborted) {
      if (!reconnect_requested_ && !io_context_.stopped()) {
        LogWarn("MQTT 接收流程被中止，準備重新連線");
        RequestReconnect(ec);
      }
      co_return;
    }

    if (ec) {
      LogErrorFormat("接收 MQTT 訊息失敗: {}", ec.message());
      RequestReconnect(ec);
      co_return;
    }

    if (topic != cmd_topic_) {
      LogWarnFormat("收到未知 Topic {} 的訊息，忽略", topic);
      continue;
    }

    folly::dynamic json;
    try {
      json = folly::parseJson(raw_payload);
    } catch (const folly::json::parse_error&) {
      LogWarnFormat("解析 MQTT 指令失敗，payload={}", raw_payload);
      continue;
    }
    if (!json.isObject()) {
      LogWarnFormat("解析 MQTT 指令失敗，payload={}", raw_payload);
      continue;
    }

    const auto* trace_ptr = json.get_ptr("trace_id");
    const auto* code_ptr = json.get_ptr("code");

    const std::string trace_value =
        trace_ptr != nullptr && trace_ptr->isString()
        ? trace_ptr->asString()
        : "";
    const std::string code_field_value = ExtractCodeFieldValue(code_ptr);

    if (trace_ptr == nullptr || !trace_ptr->isString() || code_ptr == nullptr) {
      LogWarnFormat("MQTT 指令欄位不完整，payload={}", raw_payload);
      CommandMessage command{trace_value, code_field_value, json, json};
      co_await unsupported_handler_->Handle(command);
      continue;
    }

    std::string code_str;
    if (code_ptr->isString()) {
      code_str = code_ptr->asString();
    } else if (code_ptr->isInt()) {
      code_str = folly::to<std::string>(code_ptr->asInt());
    } else {
      LogWarnFormat("MQTT 指令 code 欄位型別不支援，payload={}", raw_payload);
      CommandMessage command{trace_value, code_field_value, json, raw_payload};
      co_await unsupported_handler_->Handle(command);
      continue;
    }

    if (code_str != "100") {
      LogInfoFormat("收到指令 {}", raw_payload);
    }

    auto handler_it = handlers_.find(code_str);
    ICommandHandler* handler =
        handler_it != handlers_.end() ? handler_it->second.get() : nullptr;

    if (handler == nullptr) {
      LogWarnFormat(
          "收到未支援的指令 code={} trace_id={}，回應錯誤",
          code_str,
          trace_value);
      CommandMessage command{trace_value, code_str, json, raw_payload};
      co_await unsupported_handler_->Handle(command);
      continue;
    }

    CommandMessage command{
        trace_value, code_str, *json.get_ptr("payload"), json};
    co_await handler->Handle(command);
  }
}
