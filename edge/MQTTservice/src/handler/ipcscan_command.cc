#include "handler/ipcscan_command.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>

IpcscanCommand::IpcscanCommand(std::shared_ptr<IIpcscanClient> client,
                               std::shared_ptr<IResultUploader> uploader)
    : client_(std::move(client)), uploader_(std::move(uploader)) {
  if (!client_ || !uploader_) {
    throw std::invalid_argument("IpcscanCommand 初始化需要有效的依賴");
  }
}

void IpcscanCommand::execute(const std::string& topic,
                             const std::string& payload) {
  (void)payload;  // IPCscan 指令不需額外參數, 忽略內容即可。
  try {
    const std::string result = client_->scan();
    uploader_->upload_ipcscan(topic, result);
  } catch (const std::exception& ex) {
    std::cerr << "[ERROR] IPCscan 呼叫失敗: " << ex.what() << std::endl;
  }
}
