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

#pragma once

#include <folly/FBString.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/AsyncScope.h>
#include <folly/experimental/coro/Task.h>
#include <proxygen/httpserver/RequestHandler.h>

#include <memory>
#include <string>

#include "../util/http_types.hpp"

namespace redsafe::apiserver {

/**
 * @brief 處理單一連線的 RequestHandler
 */
class Session : public proxygen::RequestHandler {
 public:
  /// 建構並儲存執行緒池
  explicit Session(std::shared_ptr<folly::CPUThreadPoolExecutor> executor);

  /// 收到請求標頭
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  /// 收到請求內容
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  /// 收到請求結束
  void onEOM() noexcept override;
  /// 不處理升級
  void onUpgrade(proxygen::UpgradeProtocol) noexcept override {}
  /// 請求處理完成
  void requestComplete() noexcept override;
  /// 發生錯誤
  void onError(proxygen::ProxygenError) noexcept override;

 private:
  std::shared_ptr<folly::CPUThreadPoolExecutor> executor_;  ///< 用於處理請求的執行緒池
  std::unique_ptr<proxygen::HTTPMessage> headers_;          ///< 儲存請求標頭
  folly::fbstring body_;                                    ///< 儲存請求本文
  folly::AsyncScope scope_;                                 ///< 管理協程生命週期

  /// 解析並處理請求
  folly::coro::Task<void> process_request(HTTPRequest req);
};

}  // namespace redsafe::apiserver
