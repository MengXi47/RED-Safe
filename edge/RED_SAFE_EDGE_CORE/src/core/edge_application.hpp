#pragma once

#include "core/config.hpp"
#include "http/http_client.hpp"
#include "mqtt/mqtt_workflow.hpp"
#include "ipcscan/app/grpc_server.hpp"
#include "ipcscan/app/scan_executor.hpp"

#include <boost/asio/signal_set.hpp>

#include <functional>
#include <memory>

class EdgeApplication {
 public:
  EdgeApplication(
      boost::asio::io_context& io_context,
      MqttWorkflow::mqtt_client& mqtt_client,
      ipcscan::ScanExecutor& scan_executor,
      EdgeConfig config,
      IEdgeOnlineService& online_service);

  int Run();

 private:
  void SetupSignalHandlers();

  boost::asio::io_context& io_context_;
  MqttWorkflow::mqtt_client& mqtt_client_;
  ipcscan::ScanExecutor& scan_executor_;
  EdgeConfig config_;
  IEdgeOnlineService& online_service_;
  std::unique_ptr<grpc::Server, std::function<void(grpc::Server*)>>
      grpc_server_{};
  std::unique_ptr<boost::asio::signal_set> signals_;
  bool shutting_down_{false};
};
