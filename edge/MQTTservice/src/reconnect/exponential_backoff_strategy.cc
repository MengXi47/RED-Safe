#include "reconnect/exponential_backoff_strategy.hpp"

#include <algorithm>

ExponentialBackoffStrategy::ExponentialBackoffStrategy(
    std::chrono::seconds min_delay, std::chrono::seconds max_delay)
    : min_(std::max(std::chrono::seconds{1}, min_delay)),
      max_(std::max(min_, max_delay)) {}

std::chrono::seconds ExponentialBackoffStrategy::initial_delay() const {
  return min_;
}

std::chrono::seconds ExponentialBackoffStrategy::next_delay(
    std::chrono::seconds current) const {
  auto next = current * 2;
  if (next > max_) {
    return max_;
  }
  if (next < min_) {
    return min_;
  }
  return next;
}

