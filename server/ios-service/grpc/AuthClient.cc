#include "absl/status/status.h"
using absl::Status;
#include "AuthClient.hpp"

namespace redsafe::server::grpc {

AuthClient::AuthClient(const std::string& address) {
  stub_ = ::redsafe::grpc::UserAuthService::NewStub(
      ::grpc::CreateChannel(address, ::grpc::InsecureChannelCredentials()));
}

int AuthClient::Decode(
    const std::string& token, std::string& user_id, std::string& error) {
  ::redsafe::grpc::DecodeRequest request;
  request.set_access_token(token);
  ::grpc::ClientContext context;
  ::redsafe::grpc::DecodeResponse reply;
  auto status = stub_->DecodeAccessToken(&context, request, &reply);
  if (!status.ok()) {
    error = status.error_message();
    return 5;
  }
  if (reply.code() == 0) {
    user_id = reply.user_id();
  } else {
    error = reply.error_message();
  }
  return reply.code();
}

} // namespace redsafe::server::grpc
