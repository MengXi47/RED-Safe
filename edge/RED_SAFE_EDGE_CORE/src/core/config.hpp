#pragma once

#include <chrono>
#include <optional>
#include <string>

struct EdgeConfig {
  std::string edge_id;
  std::string version;
  std::string edge_ip;
  std::string server_base_url;
  std::string mqtt_broker;
  std::uint16_t mqtt_port;
  std::string mqtt_username;
  std::string mqtt_password;
  std::string mqtt_client_id;
  std::chrono::milliseconds heartbeat_interval{1000};
  std::chrono::milliseconds ipcscan_timeout{3000};
  int grpc_port{20001};
};

class ConfigLoader {
 public:
  EdgeConfig Load() const;

 private:
  static std::string GetEnvOrDefault(
      const char* name, std::string default_value);
  static std::optional<int> GetEnvInt(const char* name);
};
