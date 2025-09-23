#pragma once

#include "app/ipc_scanner.hpp"

#include <chrono>

namespace ipcscan {

// 啟動gRPC伺服器並阻塞等待請求
void RunGrpcServer(IpcScanner& scanner, std::chrono::milliseconds timeout,
                   int port);

}  // namespace ipcscan
