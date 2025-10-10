#pragma once

#include "handler/command_handler.hpp"
#include "handler/heartbeat_handler.hpp"
#include "handler/network_info_handler.hpp"
#include "handler/scan_handler.hpp"
#include "handler/unsupported_command_handler.hpp"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

struct HandlerRegistrationEntry {
  std::string code;
  std::function<std::unique_ptr<ICommandHandler>()> factory;
};

// 額外保存裸指標的 handler
#define MQTT_HANDLER_ENTRY_WITH_PTR(                                     \
    code_literal, handler_type, ptr_lvalue, ...)                         \
  HandlerRegistrationEntry {                                             \
    code_literal, [&]() -> std::unique_ptr<ICommandHandler> {            \
      auto handler_unique = std::make_unique<handler_type>(__VA_ARGS__); \
      ptr_lvalue = handler_unique.get();                                 \
      return handler_unique;                                             \
    }                                                                    \
  }

// 一般 handler 註冊，僅回傳 unique_ptr 供 handlers_ 儲存
#define MQTT_HANDLER_ENTRY(code_literal, handler_type, ...)   \
  HandlerRegistrationEntry {                                  \
    code_literal, [&]() -> std::unique_ptr<ICommandHandler> { \
      return std::make_unique<handler_type>(__VA_ARGS__);     \
    }                                                         \
  }

inline void RegisterDefaultHandlers(
    std::unordered_map<std::string, std::unique_ptr<ICommandHandler>>& handlers,
    HeartbeatHandler*& heartbeat_handler,
    std::unique_ptr<UnsupportedCommandHandler>& unsupported_holder,
    UnsupportedCommandHandler*& unsupported_handler,
    EdgeConfig& config,
    CommandPublishFn publish_status,
    CommandPublishFn publish_response,
    std::function<void()> reset_keepalive) {
  std::array handler_entries{
      // 心跳包
      MQTT_HANDLER_ENTRY_WITH_PTR(
          "100",
          HeartbeatHandler,
          heartbeat_handler,
          config,
          publish_status,
          publish_response,
          reset_keepalive),

      // IPC掃描
      MQTT_HANDLER_ENTRY(
          "101", ScanCommandHandler, config.ipcscan_timeout, publish_response),

      // 獲取網路配置
      MQTT_HANDLER_ENTRY("102", NetworkInfoHandler, config, publish_response),

      // TODO: 未來可在此加入更多Handler
  };

  for (auto& [code, factory] : handler_entries) {
    handlers.emplace(code, factory());
  }

  unsupported_holder =
      std::make_unique<UnsupportedCommandHandler>(publish_response);
  unsupported_handler = unsupported_holder.get();
}
