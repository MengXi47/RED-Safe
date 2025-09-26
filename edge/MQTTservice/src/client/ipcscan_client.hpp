#pragma once

#include <memory>
#include <string>

class IIpcscanClient {
 public:
  virtual ~IIpcscanClient() = default;
  virtual std::string scan() = 0;
};

