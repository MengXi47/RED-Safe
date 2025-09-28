#include "handler/edge_command_handler.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "handler/ipcscan_command.hpp"
#include "handler/status_report_command.hpp"

EdgeCommandHandler::EdgeCommandHandler(
    std::shared_ptr<IIpcscanClient> ipcscan_client,
    std::shared_ptr<IResultUploader> uploader)
    : ipcscan_client_(std::move(ipcscan_client)),
      uploader_(std::move(uploader)) {
  if (!ipcscan_client_ || !uploader_) {
    throw std::invalid_argument("EdgeCommandHandler 初始化需要有效的依賴");
  }
  register_default_commands();
}

void EdgeCommandHandler::on_message(const std::string& topic,
                                    const std::string& payload) {
  const std::string trimmed = trim(payload);
  if (trimmed.empty()) {
    std::cerr << "[WARN] 收到空的指令內容" << std::endl;
    return;
  }

  std::cout << "[INFO] 收到 MQTT 指令 topic='" << topic
            << "' payload='" << trimmed << "'" << std::endl;

  dispatch_command(trimmed, topic, trimmed);
}

std::string EdgeCommandHandler::trim(const std::string& value) const {
  const auto begin =
      std::ranges::find_if_not(value, [](unsigned char ch) {
        return std::isspace(ch);
      });
  const auto end =
      std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch);
      }).base();
  if (begin >= end) {
    return {};
  }
  return std::string(begin, end);
}

void EdgeCommandHandler::dispatch_command(const std::string& command,
                                          const std::string& topic,
                                          const std::string& payload) const {
  if (auto iter = command_map_.find(command); iter != command_map_.end()) {
    iter->second->execute(topic, payload);
  } else {
    std::cerr << "[WARN] 未知的指令碼: " << command << std::endl;
  }
}

void EdgeCommandHandler::register_default_commands() {
  command_map_.emplace(
      "100",
      std::make_shared<StatusReportCommand>(uploader_));
  command_map_.emplace(
      "101",
      std::make_shared<IpcscanCommand>(ipcscan_client_, uploader_));
}
