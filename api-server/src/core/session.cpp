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

#include "session.hpp"

#include <folly/experimental/coro/AsyncScope.h>
#include <folly/experimental/coro/Future.h>
#include <folly/experimental/coro/Task.h>

#include <memory_resource>

#include "../util/IOstream.hpp"
#include "../util/logger.hpp"
#include "controller.hpp"

namespace redsafe::apiserver {
// 建構 Session，保存執行緒池供協程使用
Session::Session(std::shared_ptr<folly::CPUThreadPoolExecutor> executor)
    : executor_(std::move(executor)) {}

// 收到 HTTP 標頭
void Session::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  headers_ = std::move(headers);
}

// 收到 HTTP 本文區塊
void Session::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  body_ += body->moveToFbString();
}

// 收到請求結束標記
void Session::onEOM() noexcept {
  std::pmr::monotonic_buffer_resource pool;
  HTTPRequest req{&pool};
  req.method = headers_->getMethodString().toStdString();
  req.target = headers_->getPathAsString();
  headers_->getHeaders().forEach(
      [&](const std::string& name, const std::string& value) { req.headers[name] = value; });
  req.body = body_;

  scope_.add(process_request(std::move(req)));  // 進入協程處理請求
}

// 在執行緒池中處理請求並回傳結果
folly::coro::Task<void> Session::process_request(HTTPRequest req) {
  auto response =
      co_await folly::coro::co_via(executor_.get(), Controller(std::move(req)).handle_request());

  downstream_->getEventBase()->runInEventBaseThread(
      [self = this, response = std::move(response)]() mutable {
        auto builder = proxygen::ResponseBuilder(self->downstream_);
        builder.status(response.status);
        for (const auto& [k, v] : response.headers) builder.header(k, v);
        builder.body(response.body).sendWithEOM();
      });

  co_return;
}

// 請求處理完成，刪除自身
void Session::requestComplete() noexcept {
  delete this;
}

// 處理過程出錯，刪除自身
void Session::onError(proxygen::ProxygenError) noexcept {
  delete this;
}
}  // namespace redsafe::apiserver
