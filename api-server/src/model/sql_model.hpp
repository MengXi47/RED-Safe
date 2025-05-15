/******************************************************************************
   Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.
  
   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.
  
   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.
  
   For licensing inquiries or to obtain a formal license, please contact:
******************************************************************************/

#pragma once

#include <string>
#include "SqlConnectionManager.hpp"

namespace redsafe::apiserver::model::sql
{
    /// 專門處理 Edge 裝置註冊的 SQL Model
    class EdgeDeviceRegistrar : public ConnectionManager
    {
    public:
        EdgeDeviceRegistrar();
        /// 在 edge_devices 表註冊或更新裝置
        /// @param serial Edge 裝置序號（RED-XXXXXXXX 格式）
        /// @param version 韌體／軟體版本
        /// @param timestamp 註冊時間 ISO 8601 格式
        /// @return 若執行成功回傳 true，否則 false
        [[nodiscard]] bool RegisterEdgeDevice(
            const std::string& serial,
            const std::string& version,
            const std::string& timestamp) const;
    private:
        static constexpr auto kTableName = "edge_devices";
    };
}