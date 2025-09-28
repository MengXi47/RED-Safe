#include "common/time.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

std::string CurrentIsoTimestamp() {
  using clock = std::chrono::system_clock;
  auto now = clock::now();
  auto time = clock::to_time_t(now);
  auto seconds = clock::from_time_t(time);
  auto subseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds);

  std::tm tm_time{};
#if defined(_WIN32)
  gmtime_s(&tm_time, &time);
#else
  gmtime_r(&time, &tm_time);
#endif

  std::ostringstream oss;
  oss << std::put_time(&tm_time, "%FT%T");
  oss << '.' << std::setw(3) << std::setfill('0') << subseconds.count();
  oss << 'Z';
  return oss.str();
}
