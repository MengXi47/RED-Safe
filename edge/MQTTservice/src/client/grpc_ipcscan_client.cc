#include "client/grpc_ipcscan_client.hpp"

#include <stdexcept>

#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>

GrpcIpcscanClient::GrpcIpcscanClient(const std::string& target)
    : stub_(::ipcscan::grpc::IPCScanService::NewStub(
          grpc::CreateChannel(target, grpc::InsecureChannelCredentials()))) {}

std::string GrpcIpcscanClient::scan() {
  ::ipcscan::grpc::ScanRequest request;
  ::ipcscan::grpc::ScanResponse response;
  grpc::ClientContext context;
  const auto status = stub_->Scan(&context, request, &response);
  if (!status.ok()) {
    throw std::runtime_error("IPCscan gRPC 呼叫失敗: " + status.error_message());
  }
  return response.result();
}
