#include "ipc_info_handler.hpp"
#include "sql/SqlCRUD.hpp"
#include "util/logging.hpp"

DelIPCInfoHandler::DelIPCInfoHandler(const CommandPublishFn& publish_response)
    : publish_response_{publish_response} {}
boost::asio::awaitable<void> DelIPCInfoHandler::Handle(
    const CommandMessage& command) {
  auto get_sv = [](const folly::dynamic* ptr) -> std::string_view {
    if (ptr && ptr->isString()) {
      return ptr->getString();
    }
    return std::string_view{};
  };

  const auto* ip_ptr = command.payload.get_ptr("ip");
  const auto ip = get_sv(ip_ptr);

  if (ip.empty()) {
    const auto res = BuildErrorResponse(
        command.trace_id, folly::to<int>(command.code), "Invalid ..");
    co_await publish_response_(res, "Error to Send delipcinfo");
    co_return;
  }

  if (!sql::IPCinfo::del(ip)) {
    const auto res = BuildErrorResponse(
        command.trace_id, folly::to<int>(command.code), "Error to del ..");
    co_await publish_response_(res, "Error to Send delipcinfo");
    co_return;
  }

  const auto res = BuildSuccessResponse(
      command.trace_id, folly::to<int>(command.code), folly::dynamic{});
  co_await publish_response_(res, "Error to Send delipcinfo");

  co_return;
}