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

#include <folly/dynamic.h>
#include <folly/json.h>

#include <initializer_list>
#include <string>

namespace redsafe::apiserver {

using json = folly::dynamic;

/// \brief 解析 JSON 字串
inline json parse_json(std::string_view s) {
  return folly::parseJson(s);
}

/// \brief 將 JSON 物件轉為字串
inline std::string to_json(const json& j) {
  return folly::toJson(j);
}

/// \brief 判斷是否含有指定鍵
inline bool contains(const json& j, folly::StringPiece key) {
  return j.isObject() && j.get_ptr(key) != nullptr;
}

/// \brief 取得字串欄位，若不存在回傳預設值
inline std::string get_string(const json& j, folly::StringPiece key, std::string def = {}) {
  if (auto p = j.get_ptr(key)) return p->asString().toStdString();
  return def;
}

/// \brief 以初始列表建立 JSON 物件
inline json make_object(std::initializer_list<std::pair<folly::StringPiece, json>> list) {
  json obj = folly::dynamic::object;
  for (auto& kv : list) obj[kv.first] = kv.second;
  return obj;
}

}  // namespace redsafe::apiserver
