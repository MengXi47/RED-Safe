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
    // Edge 裝置註冊
    class EdgeDeviceRegistrar : public ConnectionManager
    {
    public:
        explicit EdgeDeviceRegistrar(
            std::string serial,
            std::string version,
            std::string timestamp);

        [[nodiscard]] bool RegisterEdgeDevice() const;
    private:
        std::string serial_number_;
        std::string version_;
        std::string timestamp_;
        static constexpr auto kTableName = "edge_devices";
    };

    // 使用者註冊
    class UserRegistrar : public ConnectionManager
    {
    public:
        explicit UserRegistrar(
            std::string email,
            std::string user_name,
            std::string password_hash,
            std::string timestamp);

        [[nodiscard]] bool RegisterUser() const;
    private:
        std::string email_;
        std::string user_name_;
        std::string password_hash_;
        std::string timestamp_;
        static constexpr auto kTableName = "users";
    };

    class UserIDFinder : public ConnectionManager
    {
    public:
        explicit UserIDFinder(std::string email);

        [[nodiscard]] std::string FetchUserId() const;
    private:
        std::string email_;
        static constexpr auto kTableName = "users";
    };

    // IOS裝置註冊
    class IOSDeviceRegistrar : public ConnectionManager
    {
    public:
        explicit IOSDeviceRegistrar(
            std::string user_id,
            std::string apns_token,
            std::string device_name,
            std::string timestamp);

        [[nodiscard]] bool RegisterIOSDevice() const;
    private:
        std::string user_id_;
        std::string apns_token_;
        std::string device_name_;
        std::string timestamp_;
        static constexpr auto kTableName = "ios_devices";
    };

    // 取得 iOS 裝置 ID
    class IOSDeviceFinder : public ConnectionManager
    {
    public:
        explicit IOSDeviceFinder(std::string apns_token);

        [[nodiscard]] std::string FetchIOSDeviceId() const;
    private:
        std::string apns_token_;
        static constexpr auto kTableName = "ios_devices";
    };

    // Edge ↔ iOS 裝置綁定
    class EdgeIOSBindingRegistrar : public ConnectionManager
    {
    public:
        EdgeIOSBindingRegistrar(std::string edge_serial_number,
                                std::string ios_device_id);

        /// 建立綁定
        [[nodiscard]] bool Bind()   const;

        /// 解除綁定
        [[nodiscard]] bool Unbind() const;
    private:
        std::string edge_serial_number_;
        std::string ios_device_id_;
        static constexpr auto kTableName = "edge_device_ios_devices";
    };
}