#include "update_config_edge_password.hpp"

#include <util/logging.hpp>

#include "sql/SqlCRUD.hpp"

UpdateEdgePassword::UpdateEdgePassword(
    const CommandPublishFn& publish_response, std::string_view edge_id)
    : publish_response_{publish_response}, edge_id_{edge_id} {}

boost::asio::awaitable<void> UpdateEdgePassword::Handle(
    const CommandMessage& command) {
  auto get_sv = [](const folly::dynamic* ptr) -> std::string_view {
    if (ptr == nullptr) {
      return std::string_view{};
    }
    return ptr->getString();
  };

  const auto edge_password = get_sv(command.payload.get_ptr("edge_password"));

  if (edge_password.empty()) {
    LogWarn("payload 缺少必要欄位（edge_password）");
    const auto res =
        BuildErrorResponse(command.trace_id, command.code, "Invalid ..");
    co_await publish_response_(res, "FUCK to Pub");
    co_return;
  }

  const auto ok = sql::config::updateEdgePassword(edge_id_, edge_password);

  if (!ok) {
    LogWarn("UpdateEdgePassword fuck");
    const auto res =
        BuildErrorResponse(command.trace_id, command.code, "Fuck ..");
    co_await publish_response_(res, "FUCK to Pub");
    co_return;
  }

  LogInfoFormat(
      "UpdateEdgePassword: success to update {}:{}", edge_id_, edge_password);
  const auto res =
      BuildSuccessResponse(command.trace_id, command.code, folly::dynamic{});
  co_await publish_response_(res, command.trace_id);

  co_return;
}