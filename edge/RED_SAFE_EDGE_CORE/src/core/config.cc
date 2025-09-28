#include "core/config.hpp"

#include "common/logging.hpp"

#include <algorithm>
#include <cstdlib>

namespace {
constexpr int kMinHeartbeatMs = 100;
constexpr int kMinScanTimeoutMs = 500;
} // namespace

std::string ConfigLoader::GetEnvOrDefault(
    const char* name, std::string default_value) {
  if (const char* value = std::getenv(name)) {
    return value;
  }
  return default_value;
}

std::optional<int> ConfigLoader::GetEnvInt(const char* name) {
  if (const char* value = std::getenv(name)) {
    try {
      return std::stoi(value);
    } catch (const std::exception& ex) {
      LogWarnFormat(
          "環境變數 {} 無法轉為整數: {} - {}", name, value, ex.what());
    } catch (...) {
      LogWarnFormat("環境變數 {} 無法轉為整數: {}", name, value);
    }
  }
  return std::nullopt;
}

EdgeConfig ConfigLoader::Load() const {
  EdgeConfig cfg;
  cfg.edge_id = GetEnvOrDefault("RED_SAFE_EDGE_ID", "RED-AAAAAAAA");
  cfg.version = GetEnvOrDefault("RED_SAFE_EDGE_VERSION", "1.0.0");
  cfg.edge_ip = GetEnvOrDefault("RED_SAFE_EDGE_IP", "");
  // cfg.server_base_url = GetEnvOrDefault("RED_SAFE_SERVER_URL",
  // "https://api.redsafe-tw.com");
  cfg.server_base_url =
      GetEnvOrDefault("RED_SAFE_SERVER_URL", "https://127.0.0.1");
  cfg.mqtt_broker =
      GetEnvOrDefault("RED_SAFE_MQTT_BROKER", "mqtt.redsafe-tw.com/mqtt");
  cfg.mqtt_port =
      static_cast<std::uint16_t>(GetEnvInt("RED_SAFE_MQTT_PORT").value_or(443));
  cfg.mqtt_username = GetEnvOrDefault("RED_SAFE_MQTT_USERNAME", "redsafemqtt");
  cfg.mqtt_password = GetEnvOrDefault("RED_SAFE_MQTT_PASSWORD", "redsafemqtt");
  cfg.grpc_port = GetEnvInt("RED_SAFE_GRPC_PORT").value_or(20001);
  cfg.mqtt_client_id = "Client-" + cfg.edge_id;

  if (auto hb_ms = GetEnvInt("RED_SAFE_HEARTBEAT_MS")) {
    cfg.heartbeat_interval =
        std::chrono::milliseconds(std::max(kMinHeartbeatMs, *hb_ms));
  }
  if (auto scan_ms = GetEnvInt("RED_SAFE_IPCSCAN_TIMEOUT_MS")) {
    cfg.ipcscan_timeout =
        std::chrono::milliseconds(std::max(kMinScanTimeoutMs, *scan_ms));
  }
  return cfg;
}
