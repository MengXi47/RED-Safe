#include "util/logging.hpp"

#include <iostream>
#include <string>

#include "util/time.hpp"

namespace {
std::mutex& LogMutex() {
  static std::mutex mutex;
  return mutex;
}

std::string_view LevelTag(LogLevel level) {
  switch (level) {
    case LogLevel::Info:
      return "[INFO]";
    case LogLevel::Warn:
      return "[WARN]";
    case LogLevel::Error:
      return "[ERROR]";
    case LogLevel::Debug:
      return "[DEBUG]";
    default:
      return "[INFO]";
  }
}

std::ostream& LevelStream(LogLevel level) {
  if (level == LogLevel::Error) {
    return std::cerr;
  }
  return std::cout;
}

std::string IsoTimestamp() {
  return CurrentIsoTimestamp();
}

} // namespace

void LogMessage(LogLevel level, std::string_view message) {
  std::scoped_lock lock(LogMutex());
  auto& stream = LevelStream(level);
  stream << LevelTag(level) << ' ' << IsoTimestamp() << ' ' << message
         << std::endl;
}

void LogInfo(std::string_view message) {
  LogMessage(LogLevel::Info, message);
}

void LogWarn(std::string_view message) {
  LogMessage(LogLevel::Warn, message);
}

void LogError(std::string_view message) {
  LogMessage(LogLevel::Error, message);
}

void LogDebug(std::string_view message) {
  if constexpr (kDebugLogsEnabled) {
    LogMessage(LogLevel::Debug, message);
  } else {
    (void)message;
  }
}
