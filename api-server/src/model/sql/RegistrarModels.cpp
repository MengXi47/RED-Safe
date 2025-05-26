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

#include "RegistrarModels.hpp"
#include "../../util/IOstream.hpp"
#include "../../util/logger.hpp"

namespace redsafe::apiserver::model::sql::reg
{
    int EdgeDeviceRegistrar::start(
        std::string_view serial_number,
        std::string_view version
    )
    {
        try
        {
            pqxx::work tx{connection()};
            const auto r = tx.exec(
                pqxx::prepped("register_edge"),
                pqxx::params{serial_number, version}
            );
            if (r.affected_rows() == 0)
                return 1;
            tx.commit();
            return 0;
        }
        catch (const std::exception &e)
        {
            util::cerr()
                    << "EdgeDeviceRegistrar::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "EdgeDeviceRegistrar::start failed: "
                    << e.what();
            return 2;
        }
    }

    int UserRegistrar::start(
        std::string_view email,
        std::string_view user_name,
        std::string_view password_hash
    )
    {
        try
        {
            pqxx::work tx{connection()};
            const auto r = tx.exec(
                pqxx::prepped("register_user"),
                pqxx::params{email, user_name, password_hash}
            );
            if (r.affected_rows() == 0)
                return 1;
            tx.commit();
            return 0;
        }
        catch (const std::exception &e)
        {
            util::cerr()
                    << "UserRegistrar::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "UserRegistrar::start failed: "
                    << e.what();
            return 2;
        }
    }

    int IOSDeviceRegistrar::start(
        std::string_view ios_device_id,
        std::string_view user_id,
        std::string_view apns_token,
        std::string_view device_name
    )
    {
        try
        {
            pqxx::work tx{connection()};
            pqxx::params p;
            if (ios_device_id.empty())
                p.append("");
            else
                p.append(ios_device_id);
            p.append(user_id);
            p.append(apns_token);
            p.append(device_name);
            tx.exec(pqxx::prepped("register_ios_device"), p);
            tx.commit();
            return 0;
        }
        catch (const std::exception &e)
        {
            util::cerr()
                    << "IOSDeviceRegistrar::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "IOSDeviceRegistrar::start failed: "
                    << e.what();
            return 1;
        }
    }

    int EdgeIOSBindingRegistrar::bind(
        std::string_view edge_serial_number,
        std::string_view user_id
    )
    {
        try
        {
            pqxx::work tx{connection()};
            const auto r = tx.exec(
                pqxx::prepped("bind_edge_user"),
                pqxx::params{edge_serial_number, user_id}
            );
            if (r.affected_rows() == 0)
                return 1;
            tx.commit();
            return 0;
        }
        catch (const std::exception& e)
        {
            util::cerr()
                    << "EdgeIOSBindingRegistrar::bind failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "EdgeIOSBindingRegistrar::bind failed: "
                    << e.what();
            return 2;
        }
    }

    int EdgeIOSBindingRegistrar::unbind(
        std::string_view edge_serial_number,
        std::string_view user_id
    )
    {
        try
        {
            pqxx::work tx{connection()};
            tx.exec(
                pqxx::prepped("unbind_edge_user"),
                pqxx::params{edge_serial_number, user_id}
            );
            tx.commit();
            return 0;
        }
        catch (const std::exception& e)
        {
            util::cerr() << "EdgeIOSBindingRegistrar::unbind failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "EdgeIOSBindingRegistrar::unbind failed: "
                    << e.what();
            return 1;
        }
    }
}