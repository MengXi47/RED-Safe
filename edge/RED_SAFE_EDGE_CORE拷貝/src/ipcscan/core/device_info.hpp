#pragma once

#include <string>

namespace ipcscan {

struct DeviceInfo {
  std::string ip;
  std::string mac;
  std::string name;
};

}  // namespace ipcscan
