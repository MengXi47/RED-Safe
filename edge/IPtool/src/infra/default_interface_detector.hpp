#pragma once

#include <memory>
#include <string>

#include "domain/interface_detector.hpp"

namespace iptool::infra {

class CommandExecutor;

class DefaultInterfaceDetector final : public domain::InterfaceDetector {
 public:
  explicit DefaultInterfaceDetector(std::shared_ptr<CommandExecutor> executor);

  std::string ResolveInterface(const std::string& requested_interface) override;

 private:
  std::string DetectDefaultInterface();
  static std::string DetectFromLinuxRoutes();
  [[nodiscard]] std::string DetectFromMacRoutes() const;

  std::shared_ptr<CommandExecutor> executor_;
};

}  // namespace iptool::infra
