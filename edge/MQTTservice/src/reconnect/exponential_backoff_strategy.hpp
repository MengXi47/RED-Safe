#pragma once

#include <chrono>

#include "reconnect/reconnect_strategy.hpp"

class ExponentialBackoffStrategy : public IReconnectStrategy {
 public:
  ExponentialBackoffStrategy(std::chrono::seconds min_delay,
                             std::chrono::seconds max_delay);

  std::chrono::seconds initial_delay() const override;
  std::chrono::seconds next_delay(std::chrono::seconds current) const override;

 private:
  std::chrono::seconds min_;
  std::chrono::seconds max_;
};
