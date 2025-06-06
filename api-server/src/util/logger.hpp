/******************************************************************************
Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
******************************************************************************/

#ifndef REDSAFE_LOGGER_UTIL_HPP
#define REDSAFE_LOGGER_UTIL_HPP

#include <array>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

#include "timestamp.hpp"

namespace redsafe::apiserver::util {
enum class Level { INFO, WARNING, ERROR };
enum class LogFile { server, access };

constexpr std::string_view to_string(const Level lv) noexcept {
  switch (lv) {
    case Level::INFO:
      return "INFO";
    case Level::WARNING:
      return "WARNING";
    case Level::ERROR:
      return "ERROR";
  }
  return "UNKNOWN";
}

constexpr std::string_view to_string(const LogFile lg) noexcept {
  switch (lg) {
    case LogFile::server:
      return "server.log";
    case LogFile::access:
      return "access.log";
  }
  return "UNKNOWN";
}

class LoggerManager {
 public:
  static inline std::array<std::ofstream, 2> streams;
  static inline std::mutex log_mutex;

  static void init(LogFile file, const std::string &path) {
    std::lock_guard lock(log_mutex);
    streams[static_cast<size_t>(file)].open(path, std::ios::app);
  }
};

struct LogStream {
  Level lv;
  LogFile file;
  std::ostringstream oss;

  LogStream(const LogFile file, const Level level) : lv(level), file(file) {}

  template <typename T>
  LogStream &operator<<(const T &value) {
    oss << value;
    return *this;
  }

  ~LogStream() {
    std::lock_guard lock(LoggerManager::log_mutex);
    auto &ofs = LoggerManager::streams[static_cast<size_t>(file)];
    if (!ofs.is_open()) ofs.open(std::string(to_string(file)), std::ios::app);
    ofs << current_timestamp() << "[" << to_string(lv) << "] " << oss.str() << std::endl;
  }
};

inline LogStream log(const LogFile file, const Level level) { return LogStream{file, level}; }
}  // namespace redsafe::apiserver::util

#endif