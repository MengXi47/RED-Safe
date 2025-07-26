#include "absl/status/status.h"
using absl::Status;
#include "AuthServer.hpp"
#include "../service/TokenService.hpp"

namespace redsafe::server::grpc {

class AuthServer::ServiceImpl final
    : public redsafe::grpc::UserAuthService::Service {
 public:
  ::grpc::Status DecodeAccessToken(
      ::grpc::ServerContext* context,
      const redsafe::grpc::DecodeRequest* request,
      redsafe::grpc::DecodeResponse* reply) override {
    service::token::DecodeAccessToken decode(request->access_token());
    const auto code = decode.start();
    reply->set_code(code);
    if (code == 0) {
      reply->set_user_id(decode.getUserId());
    } else {
      reply->set_error_message(decode.getErrorMessage());
    }
    return ::grpc::Status::OK;
  }
};

AuthServer::AuthServer() = default;

void AuthServer::Run(const std::string& address) {
  server_thread_ = std::make_unique<std::thread>([address]() {
    ServiceImpl service;
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, ::grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<::grpc::Server> server(builder.BuildAndStart());
    server->Wait();
  });
  server_thread_->detach();
}

} // namespace redsafe::server::grpc
