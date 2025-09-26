#pragma once

#include <chrono>

class IReconnectStrategy {
 public:
  virtual ~IReconnectStrategy() = default;
  virtual std::chrono::seconds initial_delay() const = 0;
  virtual std::chrono::seconds next_delay(std::chrono::seconds current) const = 0;
};

