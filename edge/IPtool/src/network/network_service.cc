#include "network_service.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>

#include <sys/wait.h>
#include <unistd.h>

#include "util/logging.hpp"

namespace {

// 使用 shell quote 逃脫使用者輸入，避免命令注入
std::string ShellEscape(const std::string& value) {
  std::string escaped = "'";
  for (const char ch : value) {
    if (ch == '\'') {
      escaped += "'\\''";
    } else {
      escaped += ch;
    }
  }
  escaped += "'";
  return escaped;
}

struct CommandResult {
  int status = -1;
  std::string output;
};

// 封裝 std::system 呼叫並將輸出導入暫存檔案，之後再讀出內容
std::optional<CommandResult> RunCommandWithCapture(const std::string& command) {
  char tmp_template[] = "/tmp/iptoolXXXXXX";
  int fd = mkstemp(tmp_template);
  if (fd == -1) {
    LogErrorFormat("建立暫存檔失敗: {}", std::strerror(errno));
    return std::nullopt;
  }
  close(fd);

  std::string temp_path = tmp_template;
  std::string full_command = command + " > " + ShellEscape(temp_path) + " 2>&1";
  LogDebugFormat("執行命令: {}", full_command);
  int status = std::system(full_command.c_str());

  std::ifstream file(temp_path);
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string output = buffer.str();
  file.close();
  std::remove(temp_path.c_str());

  return CommandResult{status, std::move(output)};
}

std::string Trim(std::string value) {
  auto not_space = [](const unsigned char ch) { return !std::isspace(ch); };
  value.erase(value.begin(), std::ranges::find_if(value, not_space));
  value.erase(
      std::find_if(value.rbegin(), value.rend(), not_space).base(),
      value.end());
  return value;
}

bool CommandSucceeded(int status) {
  if (status == -1) {
    return false;
  }
  if (WIFEXITED(status)) {
    return WEXITSTATUS(status) == 0;
  }
  return false;
}

// 前綴轉換成子網路遮罩
std::optional<std::string> PrefixToMask(const int prefix_length) {
  if (prefix_length < 0 || prefix_length > 32) {
    return std::nullopt;
  }
  uint32_t mask = prefix_length == 0 ? 0U : 0xFFFFFFFFu << (32 - prefix_length);
  std::array<int, 4> octets{};
  for (int i = 0; i < 4; ++i) {
    octets[i] = static_cast<int>((mask >> (24 - i * 8)) & 0xFF);
  }
  std::ostringstream stream;
  stream << octets[0] << '.' << octets[1] << '.' << octets[2] << '.'
         << octets[3];
  return stream.str();
}

// 子網路遮罩轉換成前綴長度
std::optional<int> MaskToPrefix(const std::string& mask) {
  std::array<int, 4> octets{};
  char dot1 = 0;
  char dot2 = 0;
  char dot3 = 0;
  std::istringstream stream(mask);
  if (!(stream >> octets[0] >> dot1 >> octets[1] >> dot2 >> octets[2] >> dot3 >>
        octets[3])) {
    return std::nullopt;
  }
  if (dot1 != '.' || dot2 != '.' || dot3 != '.') {
    return std::nullopt;
  }

  uint32_t value = 0;
  for (int octet : octets) {
    if (octet < 0 || octet > 255) {
      return std::nullopt;
    }
    value = (value << 8) | static_cast<unsigned>(octet);
  }

  bool zero_encountered = false;
  int prefix = 0;
  for (int bit = 31; bit >= 0; --bit) {
    bool is_one = (value >> bit) & 1U;
    if (is_one) {
      if (zero_encountered) {
        return std::nullopt;
      }
      ++prefix;
    } else {
      zero_encountered = true;
    }
  }
  return prefix;
}

// 將 nmcli `ipv4.addresses` 解析成 IP 和遮罩
void ParseAddressWithPrefix(
    const std::string& value, std::string* ip, std::string* mask) {
  if (value.empty()) {
    return;
  }
  auto addresses = value;
  auto delimiter = addresses.find(';');
  if (delimiter != std::string::npos) {
    addresses = addresses.substr(0, delimiter);
  }
  auto slash = addresses.find('/');
  if (slash == std::string::npos) {
    *ip = addresses;
    return;
  }
  *ip = addresses.substr(0, slash);
  auto prefix_str = addresses.substr(slash + 1);
  try {
    int prefix_length = std::stoi(prefix_str);
    if (auto mask_opt = PrefixToMask(prefix_length)) {
      *mask = *mask_opt;
    }
  } catch (...) {
    LogWarn("解析前綴長度時發生例外");
  }
}

// 從 nmcli 輸出解析出 NetworkConfigData
std::optional<NetworkConfigData> ParseConfigOutput(
    const std::string& interface_name, const std::string& output) {
  NetworkConfigData data;
  data.interface_name = interface_name;

  std::istringstream stream(output);
  std::string line;
  while (std::getline(stream, line)) {
    auto separator = line.find(':');
    if (separator == std::string::npos) {
      continue;
    }
    auto key = line.substr(0, separator);
    auto value = line.substr(separator + 1);
    if (key == "ipv4.method") {
      if (value == "auto") {
        data.mode = iptool::NETWORK_MODE_DHCP;
      } else if (value == "manual") {
        data.mode = iptool::NETWORK_MODE_MANUAL;
      } else {
        data.mode = iptool::NETWORK_MODE_UNSPECIFIED;
      }
    } else if (key == "ipv4.addresses") {
      ParseAddressWithPrefix(value, &data.ip_address, &data.subnet_mask);
    } else if (key == "ipv4.gateway") {
      data.gateway = value;
    } else if (key == "ipv4.dns") {
      auto dns_value = value;
      auto delimiter = dns_value.find(';');
      if (delimiter != std::string::npos) {
        dns_value = dns_value.substr(0, delimiter);
      }
      data.dns = dns_value;
    }
  }

  if (data.mode == iptool::NETWORK_MODE_UNSPECIFIED) {
    LogWarn("未能解析網路模式");
  }

  return data;
}

bool FetchDhcpRuntimeConfig(
    const std::string& interface_name, NetworkConfigData* data) {
  const std::string command =
      "nmcli -t -f IP4.ADDRESS,IP4.GATEWAY,IP4.DNS device show " +
      ShellEscape(interface_name);
  auto result = RunCommandWithCapture(command);
  if (!result) {
    LogWarn("查詢 DHCP 配置時無法執行 nmcli");
    return false;
  }
  if (!CommandSucceeded(result->status)) {
    LogWarnFormat("nmcli 查詢 DHCP 配置失敗: {}", result->output);
    return false;
  }

  std::istringstream stream(result->output);
  std::string line;
  while (std::getline(stream, line)) {
    if (line.empty()) {
      continue;
    }
    auto separator = line.find(':');
    if (separator == std::string::npos) {
      continue;
    }
    auto key = Trim(line.substr(0, separator));
    auto value = Trim(line.substr(separator + 1));

    if (key.rfind("IP4.ADDRESS", 0) == 0) {
      ParseAddressWithPrefix(value, &data->ip_address, &data->subnet_mask);
    } else if (key == "IP4.GATEWAY") {
      data->gateway = value;
    } else if (key.rfind("IP4.DNS", 0) == 0) {
      auto delimiter = value.find(';');
      if (delimiter != std::string::npos) {
        value = value.substr(0, delimiter);
      }
      if (data->dns.empty()) {
        data->dns = value;
      } else {
        data->dns += ';' + value;
      }
    }
  }

  if (data->ip_address.empty()) {
    LogWarnFormat("介面 {} 未取得 DHCP IP 位址", interface_name);
    return false;
  }
  return true;
}

// 透過裝置名稱查找對應的 nmcli 連線名稱 (NAME 欄位)
std::optional<std::string> ResolveConnectionName(
    const std::string& interface_name) {
  const std::string command = "nmcli -t -f NAME,DEVICE connection show";
  auto result = RunCommandWithCapture(command);
  if (!result) {
    LogError("查詢連線列表時無法執行 nmcli");
    return std::nullopt;
  }
  if (!CommandSucceeded(result->status)) {
    LogErrorFormat("nmcli 查詢連線列表失敗: {}", result->output);
    return std::nullopt;
  }

  std::istringstream stream(result->output);
  std::string line;
  while (std::getline(stream, line)) {
    if (line.empty()) {
      continue;
    }
    auto separator = line.find(':');
    if (separator == std::string::npos) {
      continue;
    }
    auto name = Trim(line.substr(0, separator));
    auto device = Trim(line.substr(separator + 1));
    if (device == interface_name) {
      return name;
    }
  }

  LogWarnFormat("找不到介面 {} 對應的 nmcli 連線名稱", interface_name);
  return std::nullopt;
}

} // namespace

std::optional<NetworkConfigData> NetworkService::GetNetworkConfig(
    const std::string& interface_name) {
  const auto connection_name = ResolveConnectionName(interface_name);
  if (!connection_name) {
    return std::nullopt;
  }

  const std::string command =
      "nmcli -t -f ipv4.method,ipv4.addresses,ipv4.gateway,ipv4.dns connection show " +
      ShellEscape(*connection_name);
  auto result = RunCommandWithCapture(command);
  if (!result) {
    LogError("取得網路設定時無法執行 nmcli");
    return std::nullopt;
  }
  if (!CommandSucceeded(result->status)) {
    LogErrorFormat("nmcli 取得設定失敗: {}", result->output);
    return std::nullopt;
  }

  auto config = ParseConfigOutput(interface_name, result->output);
  if (!config) {
    LogError("無法解析 nmcli 輸出");
    return std::nullopt;
  }
  if (config->mode == iptool::NETWORK_MODE_DHCP) {
    FetchDhcpRuntimeConfig(interface_name, &*config);
  }
  return config;
}

OperationResult NetworkService::SetManualConfig(
    const NetworkConfigData& config) {
  OperationResult result;

  if (config.interface_name.empty()) {
    result.message = "介面名稱不得為空";
    return result;
  }
  if (config.ip_address.empty()) {
    result.message = "請提供 IP 位址";
    return result;
  }
  if (config.subnet_mask.empty()) {
    result.message = "請提供子網路遮罩";
    return result;
  }
  if (config.dns.empty()) {
    result.message = "請提供 DNS 伺服器";
    return result;
  }

  auto prefix_opt = MaskToPrefix(config.subnet_mask);
  if (!prefix_opt) {
    result.message = "子網路遮罩格式錯誤";
    return result;
  }

  auto connection_name = ResolveConnectionName(config.interface_name);
  if (!connection_name) {
    result.message = "找不到對應的連線名稱";
    return result;
  }

  std::string address_with_prefix =
      config.ip_address + "/" + std::to_string(*prefix_opt);
  std::string command = "nmcli connection modify " +
      ShellEscape(*connection_name) + " ipv4.method manual";
  command += " ipv4.addresses " + ShellEscape(address_with_prefix);
  command += " ipv4.gateway " + ShellEscape(config.gateway);
  command += " ipv4.dns " + ShellEscape(config.dns);

  auto modify_result = RunCommandWithCapture(command);
  if (!modify_result) {
    result.message = "無法執行 nmcli 修改指令";
    return result;
  }
  if (!CommandSucceeded(modify_result->status)) {
    result.message = modify_result->output;
    LogErrorFormat("手動設定指令失敗: {}", modify_result->output);
    return result;
  }

  const std::string up_command =
      "nmcli connection up " + ShellEscape(*connection_name);
  auto up_result = RunCommandWithCapture(up_command);
  if (!up_result) {
    result.message = "無法執行 nmcli 啟動指令";
    return result;
  }
  if (!CommandSucceeded(up_result->status)) {
    result.message = up_result->output;
    LogErrorFormat("nmcli connection up 失敗: {}", up_result->output);
    return result;
  }

  result.success = true;
  result.message = "成功設定手動 IP";
  LogInfoFormat("介面 {} 已設定為手動 IP 模式", config.interface_name);
  return result;
}

OperationResult NetworkService::SwitchToDhcp(
    const std::string& interface_name) {
  OperationResult result;
  if (interface_name.empty()) {
    result.message = "介面名稱不得為空";
    return result;
  }

  auto connection_name = ResolveConnectionName(interface_name);
  if (!connection_name) {
    result.message = "找不到對應的連線名稱";
    return result;
  }

  std::string command = "nmcli connection modify " +
      ShellEscape(*connection_name) +
      " ipv4.method auto ipv4.addresses '' ipv4.gateway '' ipv4.dns ''";
  auto modify_result = RunCommandWithCapture(command);
  if (!modify_result) {
    result.message = "無法執行 nmcli 修改指令";
    return result;
  }
  if (!CommandSucceeded(modify_result->status)) {
    result.message = modify_result->output;
    LogErrorFormat("切換 DHCP 指令失敗: {}", modify_result->output);
    return result;
  }

  const std::string up_command =
      "nmcli connection up " + ShellEscape(*connection_name);
  auto up_result = RunCommandWithCapture(up_command);
  if (!up_result) {
    result.message = "無法執行 nmcli 啟動指令";
    return result;
  }
  if (!CommandSucceeded(up_result->status)) {
    result.message = up_result->output;
    LogErrorFormat("nmcli connection up 失敗: {}", up_result->output);
    return result;
  }

  result.success = true;
  result.message = "成功切換為 DHCP";
  LogInfoFormat("介面 {} 已切換為 DHCP", interface_name);
  return result;
}
