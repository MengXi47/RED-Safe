#pragma once

#include "core/device_info.hpp"

#include <string_view>

namespace ipcscan {

class IResponseParser {
 public:
  virtual ~IResponseParser() = default;
  // 解析回應內容並填入裝置資訊
  virtual void Parse(std::string_view xml, DeviceInfo& device) const = 0;
};

}  // namespace ipcscan
