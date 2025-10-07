#include "grpc_server.hpp"

#include "ipcscan/app/scan_executor.hpp"
#include "ipcscan.grpc.pb.h"
#include "util/logging.hpp"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <format>
#include <memory>
#include <stdexcept>
#include <string>

namespace ipcscan {
namespace {

// gRPC IPCscan 實作，接收請求時同步觸發掃描
class IpcScanServiceImpl final : public grpc::IPCScanService::Service {
 public:
  explicit IpcScanServiceImpl(ScanExecutor& executor) : executor_(executor) {}

  ::grpc::Status Scan(
      ::grpc::ServerContext*,
      const grpc::ScanRequest*,
      grpc::ScanResponse* response) override {
    LogInfo("收到 IPCscan gRPC 請求");
    try {
      std::string result = executor_.RunScan();
      response->set_result(result);
      return ::grpc::Status::OK;
    } catch (const std::exception& ex) {
      LogErrorFormat("IPCscan gRPC 執行失敗: {}", ex.what());
      return {::grpc::StatusCode::INTERNAL, ex.what()};
    } catch (...) {
      LogError("IPCscan gRPC 執行發生未知錯誤");
      return {::grpc::StatusCode::INTERNAL, "unknown error"};
    }
  }

 private:
  ScanExecutor& executor_;
};

} // namespace

// 建立並啟動 IPCscan gRPC 伺服器
GrpcServerPtr StartGrpcServer(ScanExecutor& executor, int port) {
  ::grpc::EnableDefaultHealthCheckService(true);
  ::grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  auto service = std::make_shared<IpcScanServiceImpl>(executor);

  ::grpc::ServerBuilder builder;
  const std::string address = std::format("0.0.0.0:{}", port);
  builder.AddListeningPort(address, ::grpc::InsecureServerCredentials());
  builder.RegisterService(service.get());

  auto server = builder.BuildAndStart();
  if (!server) {
    throw std::runtime_error("無法啟動 IPCscan gRPC 伺服器");
  }

  LogInfoFormat("IPCscan gRPC 伺服器已啟動，監聽 {}", address);

  auto* raw_server = server.release();
  GrpcServerPtr guarded(raw_server, [service](::grpc::Server* s) {
    if (s != nullptr) {
      s->Shutdown();
      s->Wait();
      delete s;
    }
    // service shared_ptr 會在此析構保持生命週期
    (void)service;
  });
  return guarded;
}

} // namespace ipcscan
