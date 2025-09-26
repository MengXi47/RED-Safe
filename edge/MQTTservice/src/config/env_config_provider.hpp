#pragma once

#include "config/mqtt_config.hpp"

class EnvConfigProvider : public IConfigProvider {
 public:
  MqttConfig load() const override;
};
