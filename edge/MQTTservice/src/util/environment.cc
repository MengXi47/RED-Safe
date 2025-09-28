#include "util/environment.hpp"

#include <cstdlib>
#include <iostream>

namespace util::env {

// 功能: 讀取環境變數字串, 若不存在回傳空值。
std::optional<std::string> get_env(const char* key) {
  if (const char* value = std::getenv(key); value != nullptr && *value != '\0') {
    return std::string{value};
  }
  return std::nullopt;
}

// 功能: 讀取環境變數字串, 不存在時回傳備援值。
std::string get_env_or(const char* key, const std::string& fallback) {
  if (auto value = get_env(key)) {
    return *value;
  }
  return fallback;
}

// 功能: 將環境變數解讀為布林輸出, 不存在時使用預設值。
bool get_env_bool(const char* key, bool fallback) {
  if (auto value = get_env(key)) {
    if (*value == "0" || *value == "false" || *value == "FALSE") {
      return false;
    }
    return true;
  }
  return fallback;
}

// 功能: 將環境變數轉換為秒數, 解析失敗時回傳預設秒數。
std::chrono::seconds get_env_seconds(const char* key,
                                     std::chrono::seconds fallback) {
  if (auto value = get_env(key)) {
    try {
      return std::chrono::seconds{std::stoll(*value)};
    } catch (...) {
      std::cerr << "[WARN] " << key << " 不是合法整數，使用預設值" << std::endl;
    }
  }
  return fallback;
}

}  // namespace util::env
