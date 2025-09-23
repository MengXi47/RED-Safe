#include "app/grpc_server.hpp"
#include "app/ipc_scanner.hpp"
#include "network/mac_resolver.hpp"
#include "parser/onvif_response_parser.hpp"

#include <chrono>
#include <cstdio>
#include <exception>
#include <print>

int main() {
  try {
    // 建立掃描所需元件
    ipcscan::OnvifResponseParser parser;
    ipcscan::MacResolver mac_resolver;
    ipcscan::IpcScanner scanner(parser, mac_resolver);

    std::println("IPCscan gRPC伺服器啟動於埠 20001，等待掃描請求...");
    std::fflush(stdout);
    ipcscan::RunGrpcServer(scanner, std::chrono::seconds(3), 20001);
  } catch (const std::exception& ex) {
    std::println("錯誤: {}", ex.what());
    return 1;
  }

  return 0;
}
