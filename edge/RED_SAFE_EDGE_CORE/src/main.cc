#include "app/edge_application.hpp"
#include "config/env_config_provider.hpp"
#include "ipcscan/executor.hpp"
#include "util/locking_stream.hpp"

#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>
#include <string_view>

namespace {
std::atomic_bool keep_running{true};

void signal_handler(int signal) {
  cerr{} << "\n[INFO] Caught signal " << signal << ", shutting down..." << std::endl;
  keep_running = false;
}

bool has_flag(int argc, char* argv[], std::string_view flag) {
  for (int i = 1; i < argc; ++i) {
    if (std::string_view{argv[i]} == flag) {
      return true;
    }
  }
  return false;
}

void print_usage(const char* program) {
  std::cout << "Usage: " << program << " [--ipcscan]" << std::endl;
}

}  // namespace

int main(int argc, char* argv[]) {
  try {
    EnvConfigProvider provider;
    auto config = provider.load();

    if (has_flag(argc, argv, "--help")) {
      print_usage(argv[0]);
      return 0;
    }

    if (has_flag(argc, argv, "--ipcscan")) {
      IpcScanExecutor executor(config.ipcscan.discovery_timeout);
      const auto json_result = executor.run_scan_json();
      std::cout << json_result << std::endl;
      return 0;
    }

    std::signal(SIGINT, signal_handler);
#ifdef SIGTERM
    std::signal(SIGTERM, signal_handler);
#endif

    EdgeApplication app(std::move(config));
    return app.run(keep_running);
  } catch (const std::exception& ex) {
    cerr{} << "[ERROR] RED_SAFE_EDGE_CORE 發生錯誤: " << ex.what()
           << std::endl;
    return 1;
  }
}

