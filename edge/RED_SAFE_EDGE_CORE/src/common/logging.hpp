#pragma once

#include <format>
#include <mutex>
#include <string_view>
#include <tuple>

enum class LogLevel { Info, Warn, Error, Debug };

#if defined(NDEBUG)
constexpr bool kDebugLogsEnabled = false;
#else
constexpr bool kDebugLogsEnabled = true;
#endif

void LogMessage(LogLevel level, std::string_view message);

void LogInfo(std::string_view message);
void LogWarn(std::string_view message);
void LogError(std::string_view message);
void LogDebug(std::string_view message);

template <typename... Args>
void LogFormat(LogLevel level, std::string_view fmt, Args&&... args) {
  std::string formatted;
  try {
    auto args_pack = std::make_tuple(std::forward<Args>(args)...);
    formatted = std::apply(
        [&](auto&... inner) {
          return std::vformat(
              fmt, std::make_format_args<std::format_context>(inner...));
        },
        args_pack);
  } catch (...) {
    formatted = "[format-error]";
  }
  LogMessage(level, formatted);
}

template <typename... Args>
void LogInfoFormat(std::string_view fmt, Args&&... args) {
  LogFormat(LogLevel::Info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void LogWarnFormat(std::string_view fmt, Args&&... args) {
  LogFormat(LogLevel::Warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void LogErrorFormat(std::string_view fmt, Args&&... args) {
  LogFormat(LogLevel::Error, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void LogDebugFormat(std::string_view fmt, Args&&... args) {
  if constexpr (kDebugLogsEnabled) {
    LogFormat(LogLevel::Debug, fmt, std::forward<Args>(args)...);
  } else {
    (void)fmt;
    ((void)args, ...);
  }
}
