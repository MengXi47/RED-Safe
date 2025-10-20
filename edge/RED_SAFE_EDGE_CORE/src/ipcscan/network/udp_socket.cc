#include "ipcscan/network/udp_socket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace ipcscan {

namespace {
constexpr size_t kBufferSize = 64 * 1024;
}

UdpSocket::UdpSocket() : handle_(::socket(AF_INET, SOCK_DGRAM, 0)) {
  if (handle_ < 0) {
    throw std::runtime_error("無法建立UDP socket");
  }
}

UdpSocket::~UdpSocket() {
  if (handle_ >= 0) {
    ::close(handle_);
  }
}

UdpSocket::UdpSocket(int handle) : handle_(handle) {}

UdpSocket::UdpSocket(UdpSocket&& other) noexcept : handle_(other.handle_) {
  other.handle_ = -1;
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
  if (this != &other) {
    if (handle_ >= 0) {
      ::close(handle_);
    }
    handle_ = other.handle_;
    other.handle_ = -1;
  }
  return *this;
}

void UdpSocket::EnableReuse() const {
  int reuse = 1;
  ::setsockopt(handle_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
}

void UdpSocket::SetMulticastTtl(unsigned char ttl) const {
  ::setsockopt(handle_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
}

void UdpSocket::SetReceiveTimeout(std::chrono::milliseconds timeout) const {
  timeval tv{
      .tv_sec = static_cast<time_t>(timeout.count() / 1000),
      .tv_usec = static_cast<suseconds_t>((timeout.count() % 1000) * 1000),
  };
  ::setsockopt(handle_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

void UdpSocket::Bind(uint16_t port) const {
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (::bind(handle_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    throw std::runtime_error("無法綁定UDP socket");
  }
}

void UdpSocket::SendTo(std::string_view data, const sockaddr_in& dest) const {
  auto sent = ::sendto(
      handle_,
      data.data(),
      data.size(),
      0,
      reinterpret_cast<const sockaddr*>(&dest),
      sizeof(dest));
  if (sent < 0 || static_cast<size_t>(sent) != data.size()) {
    throw std::runtime_error("傳送UDP封包失敗");
  }
}

std::optional<std::pair<std::string, std::string>> UdpSocket::ReceiveFrom()
    const {
  sockaddr_in sender{};
  socklen_t sender_len = sizeof(sender);
  char buffer[kBufferSize];

  auto received = ::recvfrom(
      handle_,
      buffer,
      sizeof(buffer),
      0,
      reinterpret_cast<sockaddr*>(&sender),
      &sender_len);
  if (received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return std::nullopt;
    }
    return std::nullopt;
  }

  char ip_buffer[INET_ADDRSTRLEN] = {0};
  ::inet_ntop(AF_INET, &sender.sin_addr, ip_buffer, sizeof(ip_buffer));
  return std::make_pair(
      std::string{ip_buffer},
      std::string{buffer, static_cast<size_t>(received)});
}

} // namespace ipcscan
