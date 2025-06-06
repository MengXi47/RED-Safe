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

#include <memory>

namespace redsafe::apiserver {
/**
 * @brief 伺服器主體，管理 Proxygen HTTPServer 實例
 */
class Server {
 public:
  /**
   * @brief 以指定埠號建構伺服器
   * @param port 監聽埠號
   */
  explicit Server(uint16_t port);

  /** @brief 解構並釋放資源 */
  ~Server();

  /** @brief 啟動伺服器 */
  void start() const;

  /** @brief 停止伺服器 */
  void stop() const;

 private:
  /** @brief 真正實作細節的類別 */
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace redsafe::apiserver
