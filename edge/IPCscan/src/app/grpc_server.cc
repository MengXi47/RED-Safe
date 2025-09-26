#include "app/grpc_server.hpp"

#include "network/ws_discovery_client.hpp"
#include "ipcscan.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

#include <cstdio>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <print>

namespace ipcscan {
namespace {

// 將字串內容轉為JSON安全格式
void AppendEscaped(std::string_view input, std::string& out) {
  for (char ch : input) {
    switch (ch) {
      case '\\':
        out.append("\\\\");
        break;
      case '\"':
        out.append("\\\"");
        break;
      case '\b':
        out.append("\\b");
        break;
      case '\f':
        out.append("\\f");
        break;
      case '\n':
        out.append("\\n");
        break;
      case '\r':
        out.append("\\r");
        break;
      case '\t':
        out.append("\\t");
        break;
      default:
        out.push_back(ch);
        break;
    }
  }
}

// 將掃描結果轉為JSON陣列字串
std::string BuildJson(const std::vector<DeviceInfo>& devices) {
  if (devices.empty()) {
    return {};
  }

  std::size_t total = 2;  // '[' + ']'
  if (devices.size() > 1) {
    total += devices.size() - 1;  // commas
  }

  for (const auto& device : devices) {
    total += 25;  // 固定欄位字樣
    total += device.ip.size();
    total += device.mac.size();
    total += device.name.size();
  }

  std::string json;
  json.reserve(total);
  json.push_back('[');
  bool first = true;
  for (const auto& device : devices) {
    if (!first) {
      json.push_back(',');
    } else {
      first = false;
    }
    json.append("{\"ip\":\"");
    AppendEscaped(device.ip, json);
    json.append("\",\"mac\":\"");
    AppendEscaped(device.mac, json);
    json.append("\",\"name\":\"");
    AppendEscaped(device.name, json);
    json.append("\"}");
  }
  json.push_back(']');
  return json;
}

class IpcScanServiceImpl final
    : public ::ipcscan::grpc::IPCScanService::Service {
 public:
  // 建構子注入掃描器
  IpcScanServiceImpl(IpcScanner& scanner, std::chrono::milliseconds timeout)
      : scanner_(scanner), timeout_(timeout) {}

  // gRPC請求進入時觸發掃描
  ::grpc::Status Scan(::grpc::ServerContext*,
                      const ::ipcscan::grpc::ScanRequest*,
                      ::ipcscan::grpc::ScanResponse* response) override {
    std::println("收到gRPC掃描請求");
    std::lock_guard<std::mutex> lock(mutex_);
    try {
      WsDiscoveryClient transport(timeout_);
      auto devices = scanner_.Scan(transport);
      if (devices.empty()) {
        response->set_result("");
      } else {
        response->set_result(BuildJson(devices));
      }
      return ::grpc::Status::OK;
    } catch (const std::exception& ex) {
      return ::grpc::Status(::grpc::StatusCode::INTERNAL, ex.what());
    }
  }

 private:
  IpcScanner& scanner_;
  std::chrono::milliseconds timeout_;
  std::mutex mutex_;
};

}  // namespace

void RunGrpcServer(IpcScanner& scanner, std::chrono::milliseconds timeout,
                   int port) {
  ::grpc::EnableDefaultHealthCheckService(true);
  ::grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  auto service = std::make_shared<IpcScanServiceImpl>(scanner, timeout);

  ::grpc::ServerBuilder builder;
  const std::string address = "0.0.0.0:" + std::to_string(port);
  builder.AddListeningPort(address, ::grpc::InsecureServerCredentials());
  builder.RegisterService(service.get());

  auto server = builder.BuildAndStart();
  if (!server) {
    throw std::runtime_error("gRPC伺服器啟動失敗");
  }

  server->Wait();
}

}  // namespace ipcscan
