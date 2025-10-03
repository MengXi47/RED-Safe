#include "infra/default_interface_detector.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "common/logging.hpp"
#include "domain/network_error.hpp"
#include "infra/command_executor.hpp"

namespace iptool::infra {

DefaultInterfaceDetector::DefaultInterfaceDetector(
    std::shared_ptr<CommandExecutor> executor)
    : executor_(std::move(executor)) {}

std::string DefaultInterfaceDetector::ResolveInterface(
    const std::string& requested_interface) {
  if (!requested_interface.empty()) {
    return requested_interface;
  }
  auto detected = DetectDefaultInterface();
  if (detected.empty()) {
    throw domain::NetworkInterfaceNotFoundError("<auto>");
  }
  LogDebugFormat("Detected default interface: {}", detected);
  return detected;
}

std::string DefaultInterfaceDetector::DetectDefaultInterface() {
#if defined(__linux__)
  return DetectFromLinuxRoutes();
#elif defined(__APPLE__)
  return DetectFromMacRoutes();
#else
  throw domain::UnsupportedPlatformError();
#endif
}

std::string DefaultInterfaceDetector::DetectFromLinuxRoutes() {
  std::ifstream route_file("/proc/net/route");
  if (!route_file.is_open()) {
    LogWarn("Unable to open /proc/net/route");
    return {};
  }

  std::string line;
  std::getline(route_file, line);  // skip header

  while (std::getline(route_file, line)) {
    std::istringstream iss(line);
    std::string iface;
    std::string destination;
    std::string gateway;
    std::string flags_str;
    if (!(iss >> iface >> destination >> gateway >> flags_str)) {
      continue;
    }
    if (destination != "00000000") {
      continue;
    }

    try {
      const auto flags = std::stoul(flags_str, nullptr, 16);
      if ((flags & 0x2U) == 0) {  // RTF_GATEWAY
        continue;
      }
    } catch (const std::exception&) {
      continue;
    }

    return iface;
  }

  return {};
}

std::string DefaultInterfaceDetector::DetectFromMacRoutes() const {
  if (!executor_) {
    LogWarn("No command executor available for macOS route detection");
    return {};
  }

  const Command command{.executable = "/usr/sbin/netstat", .arguments = {"-rn"}};
  const auto result = executor_->Execute(command);
  if (result.exit_code != 0) {
    LogWarnFormat("netstat returned non-zero exit code {}", result.exit_code);
    return {};
  }

  std::istringstream stream(result.output);
  std::string line;
  while (std::getline(stream, line)) {
    std::istringstream line_stream(line);
    std::string destination;
    line_stream >> destination;
    if (destination != "default") {
      continue;
    }

    std::string gateway;
    std::string flags;
    std::string refs;
    std::string use;
    std::string interface_name;
    line_stream >> gateway >> flags >> refs >> use >> interface_name;
    if (!interface_name.empty()) {
      return interface_name;
    }
  }

  return {};
}

}  // namespace iptool::infra
