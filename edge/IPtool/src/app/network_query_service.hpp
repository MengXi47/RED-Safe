#pragma once

#include <memory>
#include <string>

#include "domain/interface_detector.hpp"
#include "domain/network_config.hpp"
#include "domain/network_info_provider.hpp"

namespace iptool::app {

class NetworkQueryService {
 public:
  NetworkQueryService(std::shared_ptr<domain::NetworkInfoProvider> provider,
                      std::shared_ptr<domain::InterfaceDetector> detector);

  domain::NetworkConfig Fetch(const std::string& requested_interface);

 private:
  std::shared_ptr<domain::NetworkInfoProvider> provider_;
  std::shared_ptr<domain::InterfaceDetector> detector_;
};

}  // namespace iptool::app
