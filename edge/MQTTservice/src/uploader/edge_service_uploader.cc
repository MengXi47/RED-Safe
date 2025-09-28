#include "uploader/edge_service_uploader.hpp"

#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <utility>

#include <curl/curl.h>

namespace {

// 功能: 確保 CURL 全域初始化僅執行一次。
void ensure_curl_global_init() {
  static std::once_flag flag;
  std::call_once(flag, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

// 功能: 將字串轉換為安全的 JSON 字面值。
std::string escape_json(const std::string& input) {
  std::ostringstream oss;
  for (unsigned char ch : input) {
    switch (ch) {
      case '\\':
        oss << "\\\\";
        break;
      case '"':
        oss << "\\\"";
        break;
      case '\n':
        oss << "\\n";
        break;
      case '\r':
        oss << "\\r";
        break;
      case '\t':
        oss << "\\t";
        break;
      default:
        if (ch < 0x20) {
          oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
              << static_cast<int>(ch) << std::dec;
        } else {
          oss << static_cast<char>(ch);
        }
        break;
    }
  }
  return oss.str();
}

}  // namespace

EdgeServiceUploader::EdgeServiceUploader(std::string base_url)
    : base_url_(std::move(base_url)) {
  if (!base_url_.empty() && base_url_.back() == '/') {
    base_url_.pop_back();
  }
}

void EdgeServiceUploader::upload_status(const std::string& edge_id,
                                        bool online) {
  std::ostringstream body;
  body << '{'
       << "\"edge_id\":\"" << escape_json(edge_id) << "\",";
  body << "\"online\":" << (online ? "true" : "false") << '}';
  post_json("/edge/status/report", body.str());
}

void EdgeServiceUploader::upload_ipcscan(const std::string& edge_id,
                                         const std::string& result) {
  std::ostringstream body;
  body << '{'
       << "\"edge_id\":\"" << escape_json(edge_id) << "\",";
  body << "\"result\":\"" << escape_json(result) << "\"}";
  post_json("/edge/ipcscan/report", body.str());
}

void EdgeServiceUploader::post_json(const std::string& path,
                                    const std::string& body) {
  ensure_curl_global_init();

  CURL* curl = curl_easy_init();
  if (curl == nullptr) {
    std::cerr << "[ERROR] 初始化 CURL 失敗" << std::endl;
    return;
  }

  const std::string url = base_url_ + path;
  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
                   static_cast<long>(body.size()));
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "[WARN] 呼叫 " << url
              << " 失敗: " << curl_easy_strerror(res) << std::endl;
  } else {
    long http_status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
    if (http_status >= 300) {
      std::cerr << "[WARN] 呼叫 " << url
                << " 回應狀態碼: " << http_status << std::endl;
    } else {
      std::cout << "[INFO] 已將資料上傳至 " << url << std::endl;
    }
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
}
