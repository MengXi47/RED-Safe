#pragma once

#include "core/config.hpp"
#include "ipcscan/app/scan_executor.hpp"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/mqtt5/mqtt_client.hpp>

class MqttWorkflow {
public:
    using mqtt_client = boost::mqtt5::mqtt_client<
        boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>,
        boost::asio::ssl::context,
        boost::mqtt5::logger
    >;

    MqttWorkflow(mqtt_client& client, EdgeConfig config, ipcscan::ScanExecutor& executor);

    boost::asio::awaitable<void> Run();

private:
    boost::asio::awaitable<bool> SubscribeCommands();
    boost::asio::awaitable<void> PublishHeartbeat();
    boost::asio::awaitable<void> ConsumeCommands();

    std::string BuildHeartbeatPayload(std::uint64_t sequence) const;
    std::string BuildScanSuccess(const std::string& trace_id, const std::string& result_json) const;
    std::string BuildScanError(const std::string& trace_id, std::string_view error_message) const;

    mqtt_client& client_;
    EdgeConfig config_;
    ipcscan::ScanExecutor& executor_;
    std::string status_topic_;
    std::string cmd_topic_;
    std::string data_topic_;
    static constexpr auto use_nothrow_awaitable = boost::asio::as_tuple(boost::asio::deferred);
};
