#include "client/grpc_ipcscan_client.hpp"
#include "config/env_config_provider.hpp"
#include "handler/edge_command_handler.hpp"
#include "mqtt/mqtt_subscriber.hpp"
#include "reconnect/exponential_backoff_strategy.hpp"
#include "uploader/edge_service_uploader.hpp"
#include "util/environment.hpp"

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {

// 說明: 控制主執行緒是否保持運作的原子旗標。
std::atomic_bool keep_running{true};

// 功能: 處理系統訊號並觸發安全關閉流程。
void signal_handler(int signal) {
  std::cerr << "\n[INFO] Caught signal " << signal << ", shutting down..."
            << std::endl;
  keep_running = false;
}

} // namespace

// 功能: 初始化所需元件並維持 MQTT 訂閱服務的生命週期。
int main() {
  std::signal(SIGINT, signal_handler);
#ifdef SIGTERM
  std::signal(SIGTERM, signal_handler);
#endif

  try {
    EnvConfigProvider config_provider;
    auto config = config_provider.load();
    auto ipc_target =
        util::env::get_env_or("IPCSCAN_GRPC_TARGET", "localhost:20001");
    auto base_url = util::env::get_env_or(
        "EDGE_SERVICE_BASE_URL", "https://api.redsafe-tw.com");

    auto ipc_client = std::make_shared<GrpcIpcscanClient>(ipc_target);
    auto uploader = std::make_shared<EdgeServiceUploader>(base_url);
    auto handler =
        std::make_shared<EdgeCommandHandler>(ipc_client, uploader);
    auto strategy = std::make_shared<ExponentialBackoffStrategy>(
        config.reconnect_min, config.reconnect_max);

    MqttSubscriber subscriber{std::move(config), handler, strategy};
    subscriber.start();

    while (keep_running.load()) {
      std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    subscriber.stop();
  } catch (const std::exception& ex) {
    std::cerr << "[ERROR] Fatal error in MQTT subscriber: " << ex.what()
              << std::endl;
    return 1;
  }

  std::cout << "[INFO] MQTT subscriber stopped" << std::endl;
  return 0;
}
