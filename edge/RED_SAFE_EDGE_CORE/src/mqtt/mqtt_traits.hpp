#pragma once

#include <boost/asio/ssl/stream.hpp>
#include <boost/mqtt5/mqtt_client.hpp>

namespace boost::mqtt5 {

template <typename NextLayer>
struct tls_handshake_type<asio::ssl::stream<NextLayer>> {
  using type = typename asio::ssl::stream<NextLayer>::handshake_type;
  static constexpr auto client = type::client;
  static constexpr auto server = type::server;
};

template <typename NextLayer>
void assign_tls_sni(
    const authority_path& ap,
    asio::ssl::context&,
    asio::ssl::stream<NextLayer>& stream) {
  if (!ap.host.empty()) {
    ::SSL_set_tlsext_host_name(stream.native_handle(), ap.host.c_str());
  }
}

} // namespace boost::mqtt5
