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
        static void initConnection(const std::string& conn_str)
        {
            if (!conn_)
            {
                conn_ = std::make_unique<pqxx::connection>(conn_str);
                if (!conn_->is_open())
                    throw std::runtime_error("DB connection failed: " + conn_str);
                SQLinit(*conn_);
            }
        }

        static pqxx::connection& connection()
        {
            if (!conn_)
                initConnection(SQL_CONNECTION_STR);
            return *conn_;
        }
    private:
        static inline thread_local std::unique_ptr<pqxx::connection> conn_{nullptr};

        static void SQLinit(pqxx::connection& conn)
        {
            try
            {
                conn.prepare(
                    "register_edge",
                    "INSERT INTO edge_devices "
                    "(edge_serial_number, version) "
                    "VALUES ($1, $2) "
                    "ON CONFLICT (edge_serial_number) DO NOTHING"
                );
            }
            catch (const pqxx::sql_error &e)
            {
                if (e.sqlstate() != "42P05")
                    throw;
            }
        }
    };
}