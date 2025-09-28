#pragma once

#include <string>

namespace iptool::domain {

class InterfaceDetector {
 public:
  virtual ~InterfaceDetector() = default;

  virtual std::string ResolveInterface(const std::string& requested_interface) = 0;
};

}  // namespace iptool::domain
