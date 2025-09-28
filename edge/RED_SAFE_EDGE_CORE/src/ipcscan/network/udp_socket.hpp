#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

namespace ipcscan {

class UdpSocket {
 public:
  // 建立UDP socket並初始化資源
  UdpSocket();
  // 關閉socket並釋放資源
  ~UdpSocket();

  UdpSocket(const UdpSocket&) = delete;
  UdpSocket& operator=(const UdpSocket&) = delete;
  UdpSocket(UdpSocket&&) noexcept;
  UdpSocket& operator=(UdpSocket&&) noexcept;

  // 啟用位址重複使用
  void EnableReuse() const;
  // 設定多播封包的TTL
  void SetMulticastTtl(unsigned char ttl) const;
  // 設定接收超時
  void SetReceiveTimeout(std::chrono::milliseconds timeout) const;
  // 綁定到任意本地位址與指定埠
  void Bind(uint16_t port) const;

  // 傳送資料到指定目標
  void SendTo(std::string_view data, const sockaddr_in& dest) const;
  // 接收資料並回傳來源IP與內容
  std::optional<std::pair<std::string, std::string>> ReceiveFrom() const;

  // 取得原生檔案描述符
  int NativeHandle() const { return handle_; }

 private:
  explicit UdpSocket(int handle);

  int handle_ = -1;
};

} // namespace ipcscan
