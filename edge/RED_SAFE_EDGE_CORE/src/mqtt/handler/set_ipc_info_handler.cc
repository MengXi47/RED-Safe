#include "ipc_info_handler.hpp"
#include "sql/SqlCRUD.hpp"
#include "util/logging.hpp"

SetIPCInfoHandler::SetIPCInfoHandler(const CommandPublishFn& publish_response)
    : publish_response_{publish_response} {}
boost::asio::awaitable<void> SetIPCInfoHandler::Handle(
    const CommandMessage& command) {
  auto get_sv = [](const folly::dynamic* ptr) -> std::string_view {
    if (ptr == nullptr) {
      return std::string_view{};
    }
    return ptr->getString();
  };

  const auto* ip_ptr = command.payload.get_ptr("ip");
  const auto* mac_ptr = command.payload.get_ptr("mac");
  const auto* ipc_name_ptr = command.payload.get_ptr("ipc_name");
  const auto* custom_name_ptr = command.payload.get_ptr("custom_name");
  const auto* ipc_account_ptr = command.payload.get_ptr("ipc_account");
  const auto* ipc_password_ptr = command.payload.get_ptr("ipc_password");
  const auto* fall_sensitivity_ptr =
      command.payload.get_ptr("fall_sensitivity");

  const auto ip = get_sv(ip_ptr);
  const auto mac = get_sv(mac_ptr);
  const auto ipc_name = get_sv(ipc_name_ptr);
  const auto custom_name = get_sv(custom_name_ptr);
  const auto ipc_account = get_sv(ipc_account_ptr);
  const auto ipc_password = get_sv(ipc_password_ptr);
  const auto fall_sensitivity = get_sv(fall_sensitivity_ptr);

  if (ip.empty() || custom_name.empty() || fall_sensitivity.empty()) {
    LogWarn("payload 缺少必要欄位（ip/custom_name/fall_sensitivity）");
    const auto res = BuildErrorResponse(
        command.trace_id, command.code, "Invalid ..");
    co_await publish_response_(res, command.trace_id);
    co_return;
  }

  const auto ok = sql::IPCinfo::set(
      ip,
      mac,
      ipc_name,
      custom_name,
      ipc_account,
      ipc_password,
      fall_sensitivity);

  if (!ok) {
    const auto res = BuildErrorResponse(
        command.trace_id, command.code, "Failed to set ..");
    co_await publish_response_(res, command.trace_id);
    co_return;
  }

  LogInfoFormat("SetIPCInfoHandler: success to set {}", ip);
  const auto res = BuildSuccessResponse(
      command.trace_id, command.code, folly::dynamic{});
  co_await publish_response_(res, command.trace_id);

  co_return;
}