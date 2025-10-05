#include "time.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

std::string CurrentIsoTimestamp() {
  using clock = std::chrono::system_clock;
  const auto now = clock::now();
  auto time = clock::to_time_t(now);

  time += 8 * 60 * 60;

  const auto subseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - clock::from_time_t(clock::to_time_t(now)));

  std::tm tm_time{};
  gmtime_r(&time, &tm_time);

  std::ostringstream oss;
  oss << std::put_time(&tm_time, "%FT%T");
  oss << '.' << std::setw(3) << std::setfill('0') << subseconds.count();
  oss << "+08:00";
  return oss.str();
}