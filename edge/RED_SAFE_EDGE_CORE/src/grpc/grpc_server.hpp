#pragma once

#include <functional>
#include <memory>

namespace grpc {
class Server;
}

namespace ipcscan {

class ScanExecutor;

using GrpcServerPtr =
    std::unique_ptr<::grpc::Server, std::function<void(::grpc::Server*)>>;

// 啟動 gRPC 伺服器並回傳 server 物件，呼叫者負責管理 lifecycle
GrpcServerPtr StartGrpcServer(ScanExecutor& executor, int port);

} // namespace ipcscan
