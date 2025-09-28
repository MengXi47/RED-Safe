#include "config/env_config_provider.hpp"

#include <iostream>

#include "util/environment.hpp"

namespace {

// 功能: 從環境變數解析 QoS 整數值, 失敗時回傳預設數值。
int read_qos_or_default(int fallback) {
  if (auto env = util::env::get_env("MQTT_QOS")) {
    try {
      const int value = std::stoi(*env);
      if (value >= 0 && value <= 2) {
        return value;
      }
      std::cerr << "[WARN] MQTT_QOS 超出範圍 (0-2)，使用預設 " << fallback
                << '\n';
    } catch (...) {
      std::cerr << "[WARN] MQTT_QOS 不是合法整數，使用預設 " << fallback
                << '\n';
    }
  }
  return fallback;
}

}  // namespace

MqttConfig EnvConfigProvider::load() const {
  MqttConfig cfg;
  cfg.server_uri = util::env::get_env("MQTT_SERVER_URI")
                        .value_or("wss://mqtt.redsafe-tw.com:443/mqtt");
  cfg.client_id = util::env::get_env("MQTT_CLIENT_ID").value_or("TEST-MAC");
  cfg.topic = util::env::get_env("MQTT_SUB_TOPIC").value_or("RED-AAAAAAAA");
  cfg.username = util::env::get_env("MQTT_USERNAME").value_or("redsafemqtt");
  cfg.password = util::env::get_env("MQTT_PASSWORD").value_or("redsafemqtt");
  cfg.qos = read_qos_or_default(cfg.qos);
  cfg.reconnect_min =
      util::env::get_env_seconds("MQTT_RECONNECT_MIN", cfg.reconnect_min);
  cfg.reconnect_max =
      util::env::get_env_seconds("MQTT_RECONNECT_MAX", cfg.reconnect_max);
  return cfg;
}
