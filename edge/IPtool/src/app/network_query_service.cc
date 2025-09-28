#include "app/network_query_service.hpp"

#include "common/logging.hpp"

namespace iptool::app {

NetworkQueryService::NetworkQueryService(
    std::shared_ptr<domain::NetworkInfoProvider> provider,
    std::shared_ptr<domain::InterfaceDetector> detector)
    : provider_(std::move(provider)), detector_(std::move(detector)) {}

domain::NetworkConfig NetworkQueryService::Fetch(
    const std::string& requested_interface) {
  const auto resolved = detector_->ResolveInterface(requested_interface);
  LogDebugFormat("Resolved interface '{}' -> '{}'", requested_interface,
                 resolved);
  return provider_->GetConfig(resolved);
}

}  // namespace iptool::app
