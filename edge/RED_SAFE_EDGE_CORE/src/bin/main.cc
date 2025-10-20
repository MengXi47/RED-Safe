#include "core/edge_application.hpp"
#include "core/env_setting.hpp"
#include "http/http_client.hpp"
#include "ipcscan/app/scan_executor.hpp"
#include "mqtt/mqtt_workflow.hpp"
#include "util/logging.hpp"

#include <exception>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/mqtt5/logger.hpp>
#include <boost/mqtt5/mqtt_client.hpp>
#include <boost/mqtt5/websocket_ssl.hpp>
#include <curl/curl.h>

namespace asio = boost::asio;
namespace mqtt5 = boost::mqtt5;

using tcp = asio::ip::tcp;
using ssl_stream = asio::ssl::stream<tcp::socket>;
using websocket_stream = boost::beast::websocket::stream<ssl_stream>;
using mqtt_client =
    mqtt5::mqtt_client<websocket_stream, asio::ssl::context, mqtt5::logger>;

namespace {
class CurlGlobalInit final {
 public:
  CurlGlobalInit() {
    if (const auto code = curl_global_init(CURL_GLOBAL_DEFAULT); code != 0) {
      throw std::runtime_error("curl_global_init failed");
    }
  }

  ~CurlGlobalInit() { curl_global_cleanup(); }
};
} // namespace

int main() {
  try {
    CurlGlobalInit curl_guard;
    EdgeConfig config = ConfigLoader::Load();
    asio::io_context io_context;
    asio::ssl::context tls_ctx(asio::ssl::context::tls_client);
    tls_ctx.set_default_verify_paths();
    tls_ctx.set_verify_mode(asio::ssl::verify_peer);
    mqtt_client client(
        io_context, std::move(tls_ctx), mqtt5::logger{mqtt5::log_level::info});
    ipcscan::ScanExecutor scan_executor(config.ipcscan_timeout);
    CurlEdgeOnlineService online_service;
    EdgeApplication app(
        io_context, client, scan_executor, std::move(config), online_service);
    return app.Run();
  } catch (const std::exception& ex) {
    LogErrorFormat("未攔截的例外: {}", ex.what());
    return EXIT_FAILURE;
  } catch (...) {
    LogError("未攔截的未知例外");
    return EXIT_FAILURE;
  }
}
