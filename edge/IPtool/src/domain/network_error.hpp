#pragma once

#include <stdexcept>
#include <string>

namespace iptool::domain {

class NetworkError : public std::runtime_error {
 public:
  explicit NetworkError(const std::string& message)
      : std::runtime_error(message) {}
};

class NetworkInterfaceNotFoundError : public NetworkError {
 public:
  explicit NetworkInterfaceNotFoundError(const std::string& interface_name)
      : NetworkError("Interface not found: " + interface_name) {}
};

class NetworkCommandError : public NetworkError {
 public:
  NetworkCommandError(
      const std::string& command, int exit_code, const std::string& output)
      : NetworkError(
            "Command failed: " + command + " (exit code " +
            std::to_string(exit_code) + ")\n" + output),
        exit_code_(exit_code),
        command_(command) {}

  [[nodiscard]] int exit_code() const noexcept { return exit_code_; }
  [[nodiscard]] const std::string& command() const noexcept { return command_; }

 private:
  int exit_code_;
  std::string command_;
};

class UnsupportedPlatformError : public NetworkError {
 public:
  UnsupportedPlatformError() : NetworkError("Unsupported platform") {}
};

} // namespace iptool::domain
