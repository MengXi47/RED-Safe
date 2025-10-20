#include "ipcscan/network/ws_discovery_client.hpp"

#include <format>
#include <random>
#include <stdexcept>
#include <string_view>

namespace ipcscan {
namespace {
constexpr const char kMulticastAddress[] = "239.255.255.250";
constexpr uint16_t kMulticastPort = 3702;
constexpr std::string_view kWsDiscoveryUrn{
    "urn:schemas-xmlsoap-org:ws:2005:04:discovery"};
constexpr std::string_view kActionProbe{
    "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"};

std::string GenerateUuid() {
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, 15);

  auto hex = [&]() -> char {
    constexpr char digits[] = "0123456789abcdef";
    return digits[dist(rd)];
  };

  std::string uuid;
  uuid.reserve(36);

  for (int i = 0; i < 36; ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      uuid.push_back('-');
    } else if (i == 14) {
      uuid.push_back('4');
    } else if (i == 19) {
      uuid.push_back("89ab"[dist(rd) & 0x3]);
    } else {
      uuid.push_back(hex());
    }
  }
  return uuid;
}
} // namespace

WsDiscoveryClient::WsDiscoveryClient(std::chrono::milliseconds timeout)
    : timeout_(timeout) {
  socket_.EnableReuse();
  socket_.SetMulticastTtl(2);
  socket_.SetReceiveTimeout(timeout_);
  socket_.Bind(0);
}

std::vector<std::pair<std::string, std::string>> WsDiscoveryClient::Probe() {
  sockaddr_in dest{};
  dest.sin_family = AF_INET;
  dest.sin_port = htons(kMulticastPort);
  ::inet_pton(AF_INET, kMulticastAddress, &dest.sin_addr);

  auto message = BuildProbeMessage();
  socket_.SendTo(message, dest);

  std::vector<std::pair<std::string, std::string>> responses;
  while (true) {
    auto packet = socket_.ReceiveFrom();
    if (!packet) {
      break;
    }
    responses.push_back(std::move(*packet));
  }

  return responses;
}

std::string WsDiscoveryClient::BuildProbeMessage() const {
  constexpr std::string_view soap_template =
      R"(<?xml version="1.0" encoding="UTF-8"?>
<e:Envelope xmlns:e="http://www.w3.org/2003/05/soap-envelope"
            xmlns:w="http://schemas.xmlsoap.org/ws/2004/08/addressing"
            xmlns:d="http://schemas.xmlsoap.org/ws/2005/04/discovery"
            xmlns:dn="http://www.onvif.org/ver10/network/wsdl">
  <e:Header>
    <w:MessageID>uuid:{}</w:MessageID>
    <w:To>{}</w:To>
    <w:Action>{}</w:Action>
  </e:Header>
  <e:Body>
    <d:Probe>
      <d:Types>dn:NetworkVideoTransmitter</d:Types>
    </d:Probe>
  </e:Body>
</e:Envelope>)";

  return std::format(
      soap_template, GenerateUuid(), kWsDiscoveryUrn, kActionProbe);
}
} // namespace ipcscan
