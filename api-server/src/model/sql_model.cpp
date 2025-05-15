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

#include "sql_model.hpp"
#include <iostream>
#include <pqxx/pqxx>

namespace redsafe::apiserver::model::sql
{

    EdgeDeviceRegistrar::EdgeDeviceRegistrar()
    {
        auto& conn = connection();
        static bool prepared = false;
        if (!prepared)
        {
            conn.prepare(
                "register_edge",
                "INSERT INTO edge_devices "
                "(edge_serial_number, version, registered_at) "
                "VALUES ($1, $2, $3) "
                "ON CONFLICT (edge_serial_number) DO UPDATE "
                "SET version = EXCLUDED.version, "
                "    registered_at = EXCLUDED.registered_at");
            prepared = true;
        }
    }

    bool EdgeDeviceRegistrar::RegisterEdgeDevice(
        const std::string& serial,
        const std::string& version,
        const std::string& timestamp) const
    {
        try
        {
            pqxx::work tx{connection()};

            tx.exec(
                pqxx::prepped("register_edge"),
                pqxx::params{serial, version, timestamp}
            );

            tx.commit();
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "EdgeDeviceRegistrar::RegisterEdgeDevice failed: "
                      << e.what() << std::endl;
            return false;
        }
    }
}