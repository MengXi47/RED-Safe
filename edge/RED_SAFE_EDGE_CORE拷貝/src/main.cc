#include "common/logging.hpp"
#include "core/config.hpp"
#include "core/edge_application.hpp"
#include "core/http_client.hpp"
#include "core/mqtt_workflow.hpp"
#include "ipcscan/app/scan_executor.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/mqtt5/logger.hpp>
#include <boost/mqtt5/mqtt_client.hpp>
#include <boost/mqtt5/websocket_ssl.hpp>

#include <curl/curl.h>
#include <openssl/ssl.h>

#include <exception>

namespace asio = boost::asio;
namespace mqtt5 = boost::mqtt5;

using tcp = asio::ip::tcp;
using ssl_stream = asio::ssl::stream<tcp::socket>;
using websocket_stream = boost::beast::websocket::stream<ssl_stream>;
using mqtt_client = mqtt5::mqtt_client<websocket_stream, asio::ssl::context, mqtt5::logger>;

namespace boost::mqtt5 {

template <typename NextLayer>
struct tls_handshake_type<boost::asio::ssl::stream<NextLayer>> {
    using type = typename boost::asio::ssl::stream<NextLayer>::handshake_type;
    static constexpr auto client = type::client;
    static constexpr auto server = type::server;
};

template <typename NextLayer>
void assign_tls_sni(
    const authority_path& ap,
    boost::asio::ssl::context&,
    boost::asio::ssl::stream<NextLayer>& stream
) {
    if (!ap.host.empty()) {
        ::SSL_set_tlsext_host_name(stream.native_handle(), ap.host.c_str());
    }
}

} // namespace boost::mqtt5

namespace {
class CurlGlobalInit final {
public:
    CurlGlobalInit() {
        const auto code = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (code != 0) {
            throw std::runtime_error("curl_global_init failed");
        }
    }
    ~CurlGlobalInit() {
        curl_global_cleanup();
    }
};
}

int main() {
    try {
        CurlGlobalInit curl_guard;

        ConfigLoader loader;
        EdgeConfig config = loader.Load();

        asio::io_context io_context;
        asio::ssl::context tls_ctx(asio::ssl::context::tls_client);
        tls_ctx.set_default_verify_paths();
        tls_ctx.set_verify_mode(asio::ssl::verify_peer);

        mqtt_client client(io_context, std::move(tls_ctx), mqtt5::logger{mqtt5::log_level::info});

        ipcscan::ScanExecutor scan_executor(config.ipcscan_timeout);
        CurlEdgeOnlineService online_service;

        EdgeApplication app(io_context, client, scan_executor, std::move(config), online_service);
        return app.Run();
    } catch (const std::exception& ex) {
        LogErrorFormat("未攔截的例外: {}", ex.what());
        return EXIT_FAILURE;
    } catch (...) {
        LogError("未攔截的未知例外");
        return EXIT_FAILURE;
    }
}

