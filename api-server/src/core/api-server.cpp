/******************************************************************************
Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
*******************************************************************************/

#include "../../include/api-server.hpp"

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <gflags/gflags.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>

#include <memory>
#include <thread>

#include "../../config.hpp"
#include "../util/IOstream.hpp"
#include "../util/logger.hpp"
#include "session.hpp"

namespace redsafe::apiserver {
// 建立 Session 的工廠
class SessionFactory : public proxygen::RequestHandlerFactory {
 public:
  explicit SessionFactory(std::shared_ptr<folly::CPUThreadPoolExecutor> executor)
      : executor_(std::move(executor)) {}
  void onServerStart(folly::EventBase *) noexcept override {}
  void onServerStop() noexcept override {}
  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *,
                                      proxygen::HTTPMessage *) noexcept override {
    return new Session(executor_);
  }

 private:
  std::shared_ptr<folly::CPUThreadPoolExecutor> executor_;
};

// Server 的實際實作
class Server::Impl {
 public:
  explicit Impl(const uint16_t port) : addr_{folly::SocketAddress("0.0.0.0", port)} {}

  // 啟動 Proxygen 伺服器
  void start() {
    loggerinit();
    proxygen::HTTPServerOptions options;
    options.threads = FLAGS_server_threads;
    executor_ = std::make_shared<folly::CPUThreadPoolExecutor>(FLAGS_server_threads);
    options.handlerFactories =
        proxygen::RequestHandlerChain().addThen<SessionFactory>(executor_).build();
    server_ = std::make_unique<proxygen::HTTPServer>(std::move(options));
    server_->bind({addr_});
    server_->start();
    util::log(util::LogFile::server, util::Level::INFO)
        << "Server listening on port: " << addr_.getPort();
    clearandprintlogo(std::to_string(addr_.getPort()), std::to_string(FLAGS_server_threads));
  }

  // 停止伺服器
  void stop() {
    if (server_) server_->stop();
  }

 private:
  // 初始化日誌系統
  static void loggerinit() {
    util::LoggerManager::init(util::LogFile::server, SERVER_LOGFILE_PATH);
    util::LoggerManager::init(util::LogFile::access, ACCESS_LOGFILE_PATH);
  }

  // 清螢幕並印出啟動訊息與 LOGO
  static void clearandprintlogo(const std::string &port, const std::string &threadnumbers) {
    [[maybe_unused]] auto e = system("clear");
    std::cout << " _____   ______  _____            _____          __\n"
              << "|  __ \\ |  ____||  __ \\          / ____|        / _|\n"
              << "| |__) || |__   | |  | | ______ | (___    __ _ | |_   __ _\n"
              << "|  _  / |  __|  | |  | ||______| \\___ \\  / _` ||  _| / _ \\\n"
              << "| | \\ \\ | |____ | |__| |         ____) || (_| || |  | __/\n"
              << "|_|  \\_\\|______||_____/         |_____/  \\__,_||_|\\___|\n"
              << "-----------------------------------------------------------\n";
    {
      constexpr int total_width = 59;
      constexpr int inner_width = total_width - 2;
      const std::string port_msg = "Server listening on port: " + port;
      const int pad1 = (inner_width - static_cast<int>(port_msg.size())) / 2;
      const int pad2 = inner_width - pad1 - static_cast<int>(port_msg.size());
      std::cout << "-" << std::string(pad1, ' ') << port_msg << std::string(pad2, ' ') << "-"
                << "\n";
      const std::string thread_msg = "Threads: " + threadnumbers;
      const int pad3 = (inner_width - static_cast<int>(thread_msg.size())) / 2;
      const int pad4 = inner_width - pad3 - static_cast<int>(thread_msg.size());
      std::cout << "-" << std::string(pad3, ' ') << thread_msg << std::string(pad4, ' ') << "-"
                << "\n";
    }
    std::cout << "-----------------------------------------------------------\n";
  }

  folly::SocketAddress addr_;
  std::shared_ptr<folly::CPUThreadPoolExecutor> executor_;
  std::unique_ptr<proxygen::HTTPServer> server_;
};

// 建構 Server，設定監聽埠
Server::Server(const uint16_t port) : impl_{std::make_unique<Impl>(port)} {}

// 解構 Server
Server::~Server() = default;

// 啟動伺服器
void Server::start() const {
  impl_->start();
}

// 停止伺服器
void Server::stop() const {
  impl_->stop();
}
}  // namespace redsafe::apiserver
