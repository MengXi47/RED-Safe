#include <cstdlib>
#include <exception>
#include <memory>

#include "app/iptool_application.hpp"
#include "app/network_query_service.hpp"
#include "app/network_update_service.hpp"
#include "common/logging.hpp"
#include "infra/command_executor.hpp"
#include "infra/default_interface_detector.hpp"
#include "infra/dns_resolver.hpp"
#include "infra/posix_network_configurator.hpp"
#include "infra/posix_network_info_provider.hpp"

#if !defined(__APPLE__) && !defined(__linux__)
#error "IPtool only supports macOS and Linux"
#endif

int main() {
  try {
    auto executor = std::make_shared<iptool::infra::PosixCommandExecutor>();
    auto dns_resolver = std::make_shared<iptool::infra::ResolvConfDnsResolver>();
    auto detector =
        std::make_shared<iptool::infra::DefaultInterfaceDetector>(executor);
    auto info_provider = std::make_shared<iptool::infra::PosixNetworkInfoProvider>(
        dns_resolver, executor);

    auto query_service = std::make_shared<iptool::app::NetworkQueryService>(
        info_provider, detector);
    auto configurator =
        std::make_shared<iptool::infra::PosixNetworkConfigurator>(executor);
    auto update_service = std::make_shared<iptool::app::NetworkUpdateService>(
        configurator, detector);

    iptool::app::IpToolApplication application(query_service, update_service);
    return application.Run();
  } catch (const std::exception& ex) {
    LogErrorFormat("Unhandled exception: {}", ex.what());
    return EXIT_FAILURE;
  } catch (...) {
    LogError("Unhandled unknown exception");
    return EXIT_FAILURE;
  }
}
