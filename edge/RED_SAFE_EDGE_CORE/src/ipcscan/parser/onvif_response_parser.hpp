#pragma once

#include "parser/response_parser.hpp"

#include <optional>
#include <string_view>

namespace ipcscan {

class OnvifResponseParser : public IResponseParser {
 public:
  // 解析Probe回應並更新裝置資訊
  void Parse(std::string_view xml, DeviceInfo& device) const override;

 private:
  // 去除字串前後空白
  std::string_view TrimView(std::string_view input) const;
  // 擷取指定標籤內容
  std::optional<std::string_view> ExtractTag(
      std::string_view xml, std::string_view tag) const;
  // 找出Scopes欄位
  std::optional<std::string_view> ExtractScopes(std::string_view xml) const;
  // 解析Scopes內的鍵值
  void ParseScopes(std::string_view scopes, DeviceInfo& device) const;
  // 從Scopes尋找指定鍵
  std::optional<std::string_view> FindScopeValue(
      std::string_view scopes, std::string_view key) const;

  // URL解碼
  std::string PercentDecode(std::string_view value) const;
  // 格式化MAC字串
  std::string FormatMac(std::string mac) const;
};

} // namespace ipcscan
