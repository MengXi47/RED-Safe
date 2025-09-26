#pragma once

#include <string>

class IMessageHandler {
 public:
  virtual ~IMessageHandler() = default;
  virtual void on_message(const std::string& topic,
                          const std::string& payload) = 0;
};

