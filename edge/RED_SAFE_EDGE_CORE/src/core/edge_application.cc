#include "core/edge_application.hpp"

#include "common/logging.hpp"

#include <csignal>
#include <memory>
#include <boost/asio/co_spawn.hpp>

// 封裝 Edge 應用程式的主要協調邏輯
EdgeApplication::EdgeApplication(
    boost::asio::io_context& io_context,
    MqttWorkflow::mqtt_client& mqtt_client,
    ipcscan::ScanExecutor& scan_executor,
    EdgeConfig config,
    IEdgeOnlineService& online_service)
    : io_context_(io_context),
      mqtt_client_(mqtt_client),
      scan_executor_(scan_executor),
      config_(std::move(config)),
      online_service_(online_service) {}

// 啟動 Edge 應用程式，串接上線、gRPC 與 MQTT 工作流程
int EdgeApplication::Run() {
  LogInfoFormat(
      "啟動 RED_SAFE_EDGE_CORE，edge_id={} version={} mqtt_broker={} grpc_port={}",
      config_.edge_id,
      config_.version,
      config_.mqtt_broker,
      config_.grpc_port);

  if (!online_service_.ReportOnline(config_)) {
    LogError("Edge 上線流程失敗，結束程式");
    return EXIT_FAILURE;
  }

  try {
    grpc_server_ = ipcscan::StartGrpcServer(scan_executor_, config_.grpc_port);
  } catch (const std::exception& ex) {
    LogErrorFormat("啟動 gRPC 伺服器失敗: {}", ex.what());
    return EXIT_FAILURE;
  }

  SetupSignalHandlers();

  auto workflow = std::make_shared<MqttWorkflow>(
      io_context_, mqtt_client_, config_, scan_executor_, online_service_);
  boost::asio::co_spawn(
      io_context_,
      [workflow]() -> boost::asio::awaitable<void> {
        co_await workflow->Run();
      },
      [](std::exception_ptr ex) {
        if (ex) {
          try {
            std::rethrow_exception(ex);
          } catch (const std::exception& e) {
            LogErrorFormat("MQTT 工作流程拋出例外: {}", e.what());
          }
        }
      });

  io_context_.run();
  LogInfo("IoContext 已停止");

  grpc_server_.reset();
  LogInfo("程式結束");
  return EXIT_SUCCESS;
}

// 註冊系統訊號處理，確保優雅關閉
void EdgeApplication::SetupSignalHandlers() {
  signals_ =
      std::make_unique<boost::asio::signal_set>(io_context_, SIGINT, SIGTERM);
  signals_->async_wait([this](const boost::system::error_code&, int signal) {
    if (shutting_down_) {
      return;
    }
    shutting_down_ = true;
    LogWarnFormat("收到系統訊號 {}，準備關閉", signal);
    mqtt_client_.cancel();
    grpc_server_.reset();
    io_context_.stop();
  });
}
