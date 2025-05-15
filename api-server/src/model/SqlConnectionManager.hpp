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

#include "../../config.hpp"

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <stdexcept>

namespace redsafe::apiserver::model::sql
{
    class ConnectionManager
    {
    public:
        explicit ConnectionManager()
        {
            initConnection(SQL_CONNECTION_STR);
        }

        static void initConnection(const std::string& conn_str)
        {
            if (!conn_)
            {
                conn_ = std::make_unique<pqxx::connection>(conn_str);
                if (!conn_->is_open())
                    throw std::runtime_error("DB connection failed: " + conn_str);
            }
        }

        static pqxx::connection& connection()
        {
            if (!conn_)
                throw std::runtime_error("DB connection not initialized");
            return *conn_;
        }
    private:
        static inline std::unique_ptr<pqxx::connection> conn_{nullptr};
    };
}