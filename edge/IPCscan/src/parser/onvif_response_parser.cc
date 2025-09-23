#include "parser/onvif_response_parser.hpp"

#include <cctype>
#include <string>
#include <string_view>

namespace ipcscan {

namespace {
constexpr std::string_view kScopeBase{"onvif://www.onvif.org/"};
constexpr std::string_view kScopeTags[] = {"d:Scopes", "Scopes", "wsd:Scopes",
                                           "wsdd:Scopes"};
constexpr std::string_view kDelimiters{"/:=-"};

constexpr std::string_view kWhitespace{" \t\r\n"};

bool IsDelimiter(char ch) {
  return kDelimiters.find(ch) != std::string_view::npos;
}

std::size_t FindCaseInsensitive(std::string_view text,
                                std::string_view pattern) {
  if (pattern.empty() || text.size() < pattern.size()) {
    return std::string_view::npos;
  }
  const auto upper = text.size() - pattern.size();
  for (std::size_t i = 0; i <= upper; ++i) {
    bool matched = true;
    for (std::size_t j = 0; j < pattern.size(); ++j) {
      const auto lhs = static_cast<unsigned char>(text[i + j]);
      const auto rhs = static_cast<unsigned char>(pattern[j]);
      if (std::tolower(lhs) != std::tolower(rhs)) {
        matched = false;
        break;
      }
    }
    if (matched) {
      return i;
    }
  }
  return std::string_view::npos;
}
}

void OnvifResponseParser::Parse(std::string_view xml, DeviceInfo& device) const {
  if (auto scopes = ExtractScopes(xml)) {
    ParseScopes(*scopes, device);
  }
}

std::string_view OnvifResponseParser::TrimView(std::string_view input) const {
  auto begin = input.find_first_not_of(kWhitespace);
  if (begin == std::string_view::npos) {
    return {};
  }
  auto end = input.find_last_not_of(kWhitespace);
  return input.substr(begin, end - begin + 1);
}

std::optional<std::string_view> OnvifResponseParser::ExtractTag(
    std::string_view xml, std::string_view tag) const {
  std::size_t search_pos = 0;
  while ((search_pos = xml.find('<', search_pos)) != std::string_view::npos) {
    const std::size_t name_start = search_pos + 1;
    if (name_start + tag.size() > xml.size()) {
      break;
    }
    if (xml.compare(name_start, tag.size(), tag) != 0) {
      ++search_pos;
      continue;
    }

    std::size_t after_tag = name_start + tag.size();
    if (after_tag >= xml.size()) {
      break;
    }
    if (!(xml[after_tag] == '>' || std::isspace(static_cast<unsigned char>(xml[after_tag])))) {
      ++search_pos;
      continue;
    }

    auto close = xml.find('>', after_tag);
    if (close == std::string_view::npos) {
      return std::nullopt;
    }
    const std::size_t content_start = close + 1;

    std::size_t closing_search = content_start;
    while ((closing_search = xml.find("</", closing_search)) !=
           std::string_view::npos) {
      const std::size_t closing_name = closing_search + 2;
      if (closing_name + tag.size() > xml.size()) {
        return std::nullopt;
      }
      if (xml.compare(closing_name, tag.size(), tag) != 0) {
        ++closing_search;
        continue;
      }
      auto closing_end = xml.find('>', closing_name + tag.size());
      if (closing_end == std::string_view::npos) {
        return std::nullopt;
      }
      return TrimView(
          xml.substr(content_start, closing_search - content_start));
    }
    return std::nullopt;
  }
  return std::nullopt;
}

std::optional<std::string_view> OnvifResponseParser::ExtractScopes(
    std::string_view xml) const {
  for (auto tag : kScopeTags) {
    if (auto scopes = ExtractTag(xml, tag)) {
      return scopes;
    }
  }
  return std::nullopt;
}

void OnvifResponseParser::ParseScopes(std::string_view scopes,
                                      DeviceInfo& device) const {
  if (device.mac.empty()) {
    if (auto mac = FindScopeValue(scopes, "mac")) {
      device.mac = FormatMac(PercentDecode(*mac));
    }
  }
  if (device.name.empty()) {
    if (auto name = FindScopeValue(scopes, "name")) {
      device.name = PercentDecode(*name);
    }
  }
}

std::optional<std::string_view> OnvifResponseParser::FindScopeValue(
    std::string_view scopes, std::string_view key) const {
  std::size_t pos = 0;
  while (pos < scopes.size()) {
    pos = scopes.find_first_not_of(kWhitespace, pos);
    if (pos == std::string_view::npos) {
      break;
    }
    auto end = scopes.find_first_of(kWhitespace, pos);
    std::string_view token = scopes.substr(
        pos, end == std::string_view::npos ? scopes.size() - pos : end - pos);

    auto base_pos = FindCaseInsensitive(token, kScopeBase);
    if (base_pos != std::string_view::npos) {
      auto rest = token.substr(base_pos + kScopeBase.size());
      auto key_pos = FindCaseInsensitive(rest, key);
      if (key_pos != std::string_view::npos) {
        if (key_pos > 0) {
          char before = rest[key_pos - 1];
          if (!IsDelimiter(before)) {
            goto next_token;
          }
        }
        auto value = rest.substr(key_pos + key.size());
        if (!value.empty() && IsDelimiter(value.front())) {
          value.remove_prefix(1);
        }
        if (!value.empty()) {
          return TrimView(value);
        }
      }
    }

  next_token:
    if (end == std::string_view::npos) {
      break;
    }
    pos = end + 1;
  }
  return std::nullopt;
}

std::string OnvifResponseParser::PercentDecode(std::string_view value) const {
  std::string result;
  result.reserve(value.size());
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '%' && i + 2 < value.size()) {
      auto hex = value.substr(i + 1, 2);
      int high = std::isdigit(static_cast<unsigned char>(hex[0]))
                     ? hex[0] - '0'
                     : std::tolower(static_cast<unsigned char>(hex[0])) - 'a' +
                           10;
      int low = std::isdigit(static_cast<unsigned char>(hex[1]))
                    ? hex[1] - '0'
                    : std::tolower(static_cast<unsigned char>(hex[1])) - 'a' +
                          10;
      if (high >= 0 && high <= 15 && low >= 0 && low <= 15) {
        result.push_back(static_cast<char>((high << 4) | low));
        i += 2;
        continue;
      }
    }
    result.push_back(value[i] == '+' ? ' ' : value[i]);
  }
  return result;
}

std::string OnvifResponseParser::FormatMac(std::string mac) const {
  std::string filtered;
  filtered.reserve(mac.size());
  for (char ch : mac) {
    if (std::isxdigit(static_cast<unsigned char>(ch))) {
      filtered.push_back(
          static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
  }

  if (filtered.size() == 12) {
    std::string formatted;
    formatted.reserve(17);
    for (std::size_t i = 0; i < filtered.size(); ++i) {
      if (i > 0 && i % 2 == 0) {
        formatted.push_back(':');
      }
      formatted.push_back(filtered[i]);
    }
    return formatted;
  }

  for (char& ch : mac) {
    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  }
  return mac;
}

}  // namespace ipcscan
