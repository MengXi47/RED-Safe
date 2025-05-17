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
        explicit EdgeDeviceRegistrar(std::string serial, std::string version);

        [[nodiscard]] bool RegisterEdgeDevice() const;
    private:
        std::string serial_number_;
        std::string version_;
    };

    // 使用者註冊
    class UserRegistrar : public ConnectionManager
    {
    public:
        explicit UserRegistrar(
            std::string email,
            std::string user_name,
            std::string password_hash);

        [[nodiscard]] bool RegisterUser() const;
    private:
        std::string email_;
        std::string user_name_;
        std::string password_hash_;
    };

    // 取得 User ID
    class UserIDFinder : public ConnectionManager
    {
    public:
        explicit UserIDFinder(std::string email);

        [[nodiscard]] std::string FetchUserId() const;
    private:
        std::string email_;
    };

    // 取得 User name
    class UserNameFinder : public ConnectionManager
    {
    public:
        explicit UserNameFinder(std::string email);

        [[nodiscard]] std::string FetchUserName() const;
    private:
        std::string email_;
    };

    // IOS裝置註冊
    class IOSDeviceRegistrar : public ConnectionManager
    {
    public:
        IOSDeviceRegistrar(std::string ios_device_id,
                           std::string user_id,
                           std::string apns_token,
                           std::string device_name);

        [[nodiscard]] bool RegisterIOSDevice() const;
    private:
        std::string ios_device_id_;
        std::string user_id_;
        std::string apns_token_;
        std::string device_name_;
    };

    // 取得 iOS 裝置 ID
    class IOSDeviceFinder : public ConnectionManager
    {
    public:
        explicit IOSDeviceFinder(std::string apns_token);

        [[nodiscard]] std::string FetchIOSDeviceId() const;
    private:
        std::string apns_token_;
    };

    // Edge ↔ iOS 裝置綁定
    class EdgeIOSBindingRegistrar : public ConnectionManager
    {
    public:
        explicit EdgeIOSBindingRegistrar(std::string edge_serial_number, std::string user_id);

        [[nodiscard]] bool bind()   const;
        [[nodiscard]] bool unbind() const;
    private:
        std::string edge_serial_number_;
        std::string user_id;
    };

    // 依 email 取得密碼雜湊
    class UserPasswordHashFinder : public ConnectionManager
    {
    public:
        explicit UserPasswordHashFinder(std::string email);

        [[nodiscard]] std::string FetchPasswordHash() const;
    private:
        std::string email_;
    };

    // 依 user_id 取回該帳號所有 Edge 序號
    class UserEdgeListFinder : public ConnectionManager
    {
    public:
        explicit UserEdgeListFinder(std::string user_id);

        [[nodiscard]] std::vector<std::string> FetchEdges() const;
    private:
        std::string user_id_;
    };
}