#pragma once

#include <memory>
#include <string>

#include "client/ipcscan_client.hpp"
#include "handler/message_handler.hpp"
#include "uploader/result_uploader.hpp"

class EdgeCommandHandler : public IMessageHandler {
 public:
  EdgeCommandHandler(std::shared_ptr<IIpcscanClient> ipcscan_client,
                     std::shared_ptr<IResultUploader> uploader);

  void on_message(const std::string& topic,
                  const std::string& payload) override;

 private:
  std::string trim(const std::string& value) const;

  std::shared_ptr<IIpcscanClient> ipcscan_client_;
  std::shared_ptr<IResultUploader> uploader_;
};

