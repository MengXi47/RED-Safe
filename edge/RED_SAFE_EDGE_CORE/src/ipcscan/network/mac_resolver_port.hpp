#pragma once

#include <optional>
#include <string>

namespace ipcscan {

class IMacResolver {
 public:
  virtual ~IMacResolver() = default;
  // 查詢IP對應的MAC位址
  virtual std::optional<std::string> Resolve(const std::string& ip) const = 0;
};

} // namespace ipcscan
