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

#ifndef REDSAFE_FINDER_MODEL_HPP
#define REDSAFE_FINDER_MODEL_HPP

#include "SqlConnectionManager.hpp"

namespace redsafe::apiserver::model::sql::fin
{
    // 取得 User ID
    class UserIDFinder : public ConnectionManager
    {
    public:
        // 成功回傳User ID 否則空字串
        [[nodiscard]] static std::string start(std::string_view email);
    };

    // 取得 User Name
    class UserNameFinder : public ConnectionManager
    {
    public:
        // 成功回傳User Name 否則空字串
        [[nodiscard]] static std::string start(std::string_view email);
    };

    // 取得 iOS device ID
    class IOSDeviceIDFinder : public ConnectionManager
    {
    public:
        // 成功回傳iOS device ID 否則空字串
        [[nodiscard]] static std::string start(std::string_view apns_token);
    };

    // 依 email 取得密碼雜湊
    class UserPasswordHashFinder : public ConnectionManager
    {
    public:
        // 成功回傳password hash 否則空字串
        [[nodiscard]] static std::string start(std::string_view email);
    };

    // 依 user_id 取回該帳號所有 Edge 序號
    class UserEdgeListFinder : public ConnectionManager
    {
    public:
        // 成功回傳Edge 序號 ALL 否則空字串
        [[nodiscard]] static std::vector<std::string> start(std::string_view user_id);
    };
}

#endif