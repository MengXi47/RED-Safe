#pragma once

#include <memory>
#include <string>

#include <grpcpp/server.h>

#include "app/network_query_service.hpp"
#include "app/network_update_service.hpp"

namespace iptool::app {

class IpToolApplication {
 public:
  IpToolApplication(std::shared_ptr<NetworkQueryService> query_service,
                    std::shared_ptr<NetworkUpdateService> update_service,
                    std::string server_address = "0.0.0.0:20002");

  int Run();

 private:
  std::shared_ptr<NetworkQueryService> query_service_;
  std::shared_ptr<NetworkUpdateService> update_service_;
  std::string server_address_;
};

}  // namespace iptool::app
