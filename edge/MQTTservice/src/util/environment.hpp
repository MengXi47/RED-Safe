#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace util::env {

std::optional<std::string> get_env(const char* key);
std::string get_env_or(const char* key, const std::string& fallback);
bool get_env_bool(const char* key, bool fallback);
std::chrono::seconds get_env_seconds(const char* key,
                                     std::chrono::seconds fallback);

}  // namespace util::env
