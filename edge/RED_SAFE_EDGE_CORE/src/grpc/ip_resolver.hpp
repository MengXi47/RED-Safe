#pragma once

#include "core/env_setting.hpp"

#include <optional>
#include <string>

// 從 IPtool gRPC 服務查詢 Edge 裝置的介面 IP
std::optional<std::string> FetchEdgeIpFromIptool(const EdgeConfig& config);
