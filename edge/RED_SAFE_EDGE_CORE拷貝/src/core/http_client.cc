#include "core/http_client.hpp"

#include "common/logging.hpp"
#include "common/time.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

namespace {
size_t CurlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buffer = static_cast<std::string*>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}
}

std::string CurlEdgeOnlineService::BuildUrl(const std::string& base_url, std::string_view path) {
    if (base_url.empty()) {
        return std::string(path);
    }
    std::string url = base_url;
    if (url.back() == '/') {
        url.pop_back();
    }
    if (!path.empty() && path.front() != '/') {
        url.push_back('/');
    }
    url.append(path);
    return url;
}

bool CurlEdgeOnlineService::ReportOnline(const EdgeConfig& config) {
    const std::string url = BuildUrl(config.server_base_url, "/edge/online");

    nlohmann::json body{
        {"edge_id", config.edge_id},
        {"version", config.version},
        {"started_at", CurrentIsoTimestamp()}
    };
    if (!config.edge_ip.empty()) {
        body["ip"] = config.edge_ip;
    }
    const std::string payload = body.dump();

    CURL* curl = curl_easy_init();
    if (!curl) {
        LogError("初始化 CURL 失敗，無法送出上線請求");
        return false;
    }

    std::string response_body;
    long http_code = 0;

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RED_SAFE_EDGE_CORE/1.0");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    const CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LogErrorFormat("送出上線請求失敗: {}", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (http_code / 100 != 2) {
        LogErrorFormat("上線回應非 2xx，狀態碼 {}，body: {}", http_code, response_body);
        return false;
    }

    LogInfoFormat("成功通報 Edge 上線，回應狀態碼 {}", http_code);
    return true;
}
