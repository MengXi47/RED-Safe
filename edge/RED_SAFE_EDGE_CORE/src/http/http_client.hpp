#pragma once

#include "core/env_setting.hpp"

class IEdgeOnlineService {
public:
    virtual ~IEdgeOnlineService() = default;
    virtual bool ReportOnline(const EdgeConfig& config) = 0;
};

class CurlEdgeOnlineService final : public IEdgeOnlineService {
public:
    bool ReportOnline(const EdgeConfig& config) override;

private:
    static std::string BuildUrl(const std::string& base_url, std::string_view path);
};
