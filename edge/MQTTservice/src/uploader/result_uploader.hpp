#pragma once

#include <string>

class IResultUploader {
 public:
  virtual ~IResultUploader() = default;
  virtual void upload_status(const std::string& edge_id, bool online) = 0;
  virtual void upload_ipcscan(const std::string& edge_id,
                              const std::string& result) = 0;
};

