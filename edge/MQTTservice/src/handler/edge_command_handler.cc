#include "handler/edge_command_handler.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>

EdgeCommandHandler::EdgeCommandHandler(
    std::shared_ptr<IIpcscanClient> ipcscan_client,
    std::shared_ptr<IResultUploader> uploader)
    : ipcscan_client_(std::move(ipcscan_client)),
      uploader_(std::move(uploader)) {}

void EdgeCommandHandler::on_message(const std::string& topic,
                                    const std::string& payload) {
  const std::string trimmed = trim(payload);
  if (trimmed.empty()) {
    std::cerr << "[WARN] 收到空的指令內容" << std::endl;
    return;
  }

  std::cout << "[INFO] 收到 MQTT 指令 topic='" << topic
            << "' payload='" << trimmed << "'" << std::endl;

  if (trimmed == "100") {
    uploader_->upload_status(topic, true);
  } else if (trimmed == "101") {
    try {
      const std::string result = ipcscan_client_->scan();
      uploader_->upload_ipcscan(topic, result);
    } catch (const std::exception& ex) {
      std::cerr << "[ERROR] IPCscan 呼叫失敗: " << ex.what() << std::endl;
    }
  } else {
    std::cerr << "[WARN] 未知的指令碼: " << trimmed << std::endl;
  }
}

std::string EdgeCommandHandler::trim(const std::string& value) const {
  const auto begin =
      std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
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
