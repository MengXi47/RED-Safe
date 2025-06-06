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

#ifndef REDSAFE_HTTP_TYPES_HPP
#define REDSAFE_HTTP_TYPES_HPP

#include <folly/FBString.h>
#include <folly/container/F14Map.h>

#include <memory_resource>
#include <string>

namespace redsafe::apiserver {

/// 方便與記憶體池搭配的字串型別
using PmrString = std::pmr::string;
template <class K = PmrString, class V = PmrString>
/// 支援記憶體池的 F14 hash map
using PmrF14Map = folly::F14FastMap<K, V, std::hash<K>, std::equal_to<K>,
                                    std::pmr::polymorphic_allocator<std::pair<const K, V>>>;
/// 簡易的 HTTP 請求結構
struct HTTPRequest {
  PmrString method;
  PmrString target;
  PmrF14Map<> headers;
  folly::fbstring body;

  /**
   * @brief 使用指定記憶體資源初始化
   */
  explicit HTTPRequest(std::pmr::memory_resource* res = std::pmr::get_default_resource())
      : method(res), target(res), headers(res) {}
};

/// 簡易的 HTTP 回應結構
struct HTTPResponse {
  int status{200};
  PmrF14Map<> headers;
  folly::fbstring body;

  /**
   * @brief 使用指定記憶體資源初始化
   */
  explicit HTTPResponse(std::pmr::memory_resource* res = std::pmr::get_default_resource())
      : headers(res) {}
};

}  // namespace redsafe::apiserver

#endif  // REDSAFE_HTTP_TYPES_HPP
