#include "app/network_update_service.hpp"

#include "common/logging.hpp"

namespace iptool::app {

NetworkUpdateService::NetworkUpdateService(
    std::shared_ptr<domain::NetworkConfigurator> configurator,
    std::shared_ptr<domain::InterfaceDetector> detector)
    : configurator_(std::move(configurator)),
      detector_(std::move(detector)) {}

void NetworkUpdateService::Apply(domain::NetworkConfig config) {
  const auto resolved = detector_->ResolveInterface(config.interface_name);
  if (config.interface_name != resolved) {
    LogDebugFormat("Resolved update interface '{}' -> '{}'",
                   config.interface_name, resolved);
  }
  config.interface_name = resolved;
  configurator_->Apply(config);
}

}  // namespace iptool::app
