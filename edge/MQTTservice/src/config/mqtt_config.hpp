#pragma once

#include <chrono>
#include <string>

struct MqttConfig {
  std::string server_uri;
  std::string client_id;
  std::string topic;
  std::string username;
  std::string password;
  int qos{1};
  std::chrono::seconds reconnect_min{1};
  std::chrono::seconds reconnect_max{30};
};

class IConfigProvider {
 public:
  virtual ~IConfigProvider() = default;
  virtual MqttConfig load() const = 0;
};
