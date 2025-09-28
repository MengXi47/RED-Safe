#pragma once

#include <format>
#include <mutex>
#include <string_view>

enum class LogLevel {
    Info,
    Warn,
    Error,
    Debug
};

void LogMessage(LogLevel level, std::string_view message);

void LogInfo(std::string_view message);
void LogWarn(std::string_view message);
void LogError(std::string_view message);
void LogDebug(std::string_view message);

template <typename... Args>
void LogFormat(LogLevel level, std::string_view fmt, Args&&... args) {
    std::string formatted;
    try {
        formatted = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
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
    LogFormat(LogLevel::Debug, fmt, std::forward<Args>(args)...);
}
