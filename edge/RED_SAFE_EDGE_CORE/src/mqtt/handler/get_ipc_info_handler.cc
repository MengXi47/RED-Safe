#include "ipc_info_handler.hpp"

#include "sql/SqlCRUD.hpp"
#include "util/logging.hpp"

GetIPCInfoHandler::GetIPCInfoHandler(const CommandPublishFn& publish_response)
    : publish_response_(std::move(publish_response)) {}

boost::asio::awaitable<void> GetIPCInfoHandler::Handle(
    const CommandMessage& command) {
  const auto IPCinfo = sql::IPCinfo::getAll();

  if (!IPCinfo.has_value()) {
    const auto res =
        BuildErrorResponse(command.trace_id, command.code, "Error GetIPCInfo");
    co_await publish_response_(res, "Publish GetIPCInfo Error");
    co_return;
  }

  const auto res =
      BuildSuccessResponse(command.trace_id, command.code, std::move(IPCinfo.value()));
  if (co_await publish_response_(res, "Publish GetIPCInfo Error")) {
    LogInfoFormat("獲取IPCInfo結果已送出 trace_id={}", command.trace_id);
  }

  co_return;
}