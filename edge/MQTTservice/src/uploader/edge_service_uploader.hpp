#pragma once

#include <string>

#include "uploader/result_uploader.hpp"

class EdgeServiceUploader : public IResultUploader {
 public:
  explicit EdgeServiceUploader(std::string base_url);

  void upload_status(const std::string& edge_id, bool online) override;
  void upload_ipcscan(const std::string& edge_id,
                      const std::string& result) override;

 private:
  void post_json(const std::string& path, const std::string& body);
  std::string base_url_;
};

