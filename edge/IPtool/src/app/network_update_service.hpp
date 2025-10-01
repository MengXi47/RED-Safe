#pragma once

#include <memory>

#include "domain/interface_detector.hpp"
#include "domain/network_config.hpp"
#include "domain/network_configurator.hpp"

namespace iptool::app {

class NetworkUpdateService {
 public:
  NetworkUpdateService(
      std::shared_ptr<domain::NetworkConfigurator> configurator,
      std::shared_ptr<domain::InterfaceDetector> detector);

  void Apply(domain::NetworkConfig config);

 private:
  std::shared_ptr<domain::NetworkConfigurator> configurator_;
  std::shared_ptr<domain::InterfaceDetector> detector_;
};

}  // namespace iptool::app
