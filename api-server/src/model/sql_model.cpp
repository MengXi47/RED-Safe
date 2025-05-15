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
#include <utility>

namespace redsafe::apiserver::model::sql
{

    EdgeDeviceRegistrar::EdgeDeviceRegistrar(std::string serial,
                                             std::string version,
                                             std::string timestamp)
        : serial_number_(std::move(serial)),
          version_      (std::move(version)),
          timestamp_    (std::move(timestamp))
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
                "ON CONFLICT (edge_serial_number) DO NOTHING");
            prepared = true;
        }
    }

    bool EdgeDeviceRegistrar::RegisterEdgeDevice() const
    {
        try
        {
            pqxx::work tx{connection()};
            const auto r = tx.exec(
                pqxx::prepped("register_edge"),
                pqxx::params{serial_number_, version_, timestamp_}
            );
            if (r.affected_rows() == 0)
                return false;
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

    UserRegistrar::UserRegistrar(std::string email,
                                 std::string user_name,
                                 std::string password_hash,
                                 std::string timestamp)
        : email_        (std::move(email)),
          user_name_    (std::move(user_name)),
          password_hash_(std::move(password_hash)),
          timestamp_    (std::move(timestamp))
    {
        auto& conn = connection();
        static bool prepared_user = false;
        if (!prepared_user)
        {
            conn.prepare(
                "register_user",
                "INSERT INTO users "
                "(email, user_name, user_password_hash, created_at) "
                "VALUES ($1, $2, $3, $4) "
                "ON CONFLICT DO NOTHING");
            prepared_user = true;
        }
    }

    bool UserRegistrar::RegisterUser() const
    {
        try
        {
            pqxx::work tx{connection()};
            const auto r = tx.exec(
                pqxx::prepped("register_user"),
                pqxx::params{email_, user_name_, password_hash_, timestamp_}
            );
            if (r.affected_rows() == 0)
                return false;
            tx.commit();
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "UserRegistrar::RegisterUser failed: "
                      << e.what() << std::endl;
            return false;
        }
    }

    UserIDFinder::UserIDFinder(std::string email) : email_(std::move(email))
    {
        auto& conn = connection();
        static bool prepared_finder = false;
        if (!prepared_finder)
        {
            conn.prepare("find_user_id",
                "SELECT user_id FROM users WHERE email = $1");
            prepared_finder = true;
        }
    }

    std::string UserIDFinder::FetchUserId() const
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_id"),
                pqxx::params{email_}
            );
            if (r.empty())
                return std::string{};
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            std::cerr << "UserIDFinder::FetchUserId failed: "
                      << e.what() << std::endl;
            return std::string{};
        }
    }

    IOSDeviceRegistrar::IOSDeviceRegistrar(std::string user_id,
                                           std::string apns_token,
                                           std::string device_name,
                                           std::string timestamp)
        : user_id_    (std::move(user_id)),
          apns_token_ (std::move(apns_token)),
          device_name_(std::move(device_name)),
          timestamp_  (std::move(timestamp))
    {
        auto& conn = connection();
        static bool prepared_IOSDevice = false;
        if (!prepared_IOSDevice)
        {
            conn.prepare(
                "register_ios_device",
                "INSERT INTO ios_devices "
                "(user_id, apns_token, device_name, last_seen_at) "
                "VALUES ($1, $2, $3, $4) "
                "ON CONFLICT (apns_token) DO UPDATE "
                "SET user_id      = EXCLUDED.user_id, "
                "    device_name  = EXCLUDED.device_name, "
                "    last_seen_at = EXCLUDED.last_seen_at");
            prepared_IOSDevice = true;
        }
    }

    bool IOSDeviceRegistrar::RegisterIOSDevice() const
    {
        try
        {
            pqxx::work tx{connection()};
            tx.exec(
                pqxx::prepped("register_ios_device"),
                pqxx::params{user_id_, apns_token_, device_name_, timestamp_});
            tx.commit();
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "IOSDeviceRegistrar::RegisterIOSDevice failed: "
                      << e.what() << std::endl;
            return false;
        }
    }

    IOSDeviceFinder::IOSDeviceFinder(std::string apns_token) : apns_token_(std::move(apns_token))
    {
        auto& conn = connection();
        static bool prepared_finder = false;
        if (!prepared_finder)
        {
            conn.prepare("find_ios_device_id",
                "SELECT ios_device_id FROM ios_devices WHERE apns_token = $1");
            prepared_finder = true;
        }
    }

    std::string IOSDeviceFinder::FetchIOSDeviceId() const
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_ios_device_id"),
                pqxx::params{apns_token_}
            );
            if (r.empty())
                return std::string{};
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            std::cerr << "IOSDeviceFinder::FetchIOSDeviceId failed: "
                      << e.what() << std::endl;
            return std::string{};
        }
    }

    EdgeIOSBindingRegistrar::EdgeIOSBindingRegistrar(std::string edge_serial_number, std::string ios_device_id)
        : edge_serial_number_(std::move(edge_serial_number)), ios_device_id_(std::move(ios_device_id))
    {
        auto& conn = connection();
        static bool prepared_finder = false;
        if (!prepared_finder)
        {
            conn.prepare("bind_edge_ios",
                "INSERT INTO edge_device_ios_devices "
                "(edge_serial_number, ios_device_id) "
                "VALUES ($1, $2) "
                "ON CONFLICT DO NOTHING");

            conn.prepare("unbind_edge_ios",
                "DELETE FROM edge_device_ios_devices "
                "WHERE edge_serial_number = $1 "
                "AND   ios_device_id      = $2");

            prepared_finder = true;
        }
    }

    bool EdgeIOSBindingRegistrar::Bind() const
    {
        try
        {
            pqxx::work tx{connection()};
            tx.exec(
                pqxx::prepped("bind_edge_ios"),
                pqxx::params{edge_serial_number_, ios_device_id_}
            );
            tx.commit();
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "EdgeIOSBindingRegistrar::Bind failed: "
                      << e.what() << std::endl;
            return false;
        }
    }

    bool EdgeIOSBindingRegistrar::Unbind() const
    {
        try
        {
            pqxx::work tx{connection()};
            tx.exec(
                pqxx::prepped("unbind_edge_ios"),
                pqxx::params{edge_serial_number_, ios_device_id_}
            );
            tx.commit();
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "EdgeIOSBindingRegistrar::Unbind failed: "
                      << e.what() << std::endl;
            return false;
        }
    }
}
