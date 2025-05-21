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
#include <stdexcept>

namespace redsafe::apiserver::model::sql
{
    EdgeDeviceRegistrar::EdgeDeviceRegistrar(std::string serial, std::string version)
        : serial_number_(std::move(serial)), version_(std::move(version))
    {
    }

    bool EdgeDeviceRegistrar::RegisterEdgeDevice() const
    {
        try
        {
            pqxx::work tx{connection()};
            const auto r = tx.exec(
                pqxx::prepped("register_edge"),
                pqxx::params{serial_number_, version_}
            );
            if (r.affected_rows() == 0)
                throw std::runtime_error("Edge device already registered");
            tx.commit();
            return true;
        }
        catch ([[maybe_unused]] std::runtime_error& e)
        {
            throw;
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
                                 std::string password_hash)
        : email_        (std::move(email)),
          user_name_    (std::move(user_name)),
          password_hash_(std::move(password_hash))
    {
        auto& conn = connection();
        static bool prepared_user = false;
        if (!prepared_user)
        {
            conn.prepare(
                "register_user",
                "INSERT INTO users "
                "(email, user_name, user_password_hash) "
                "VALUES ($1, $2, $3) "
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
                pqxx::params{email_, user_name_, password_hash_}
            );
            if (r.affected_rows() == 0)
                throw std::runtime_error("Email already registered");
            tx.commit();
            return true;
        }
        catch ([[maybe_unused]] std::runtime_error& e)
        {
            throw;
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

    UserNameFinder::UserNameFinder(std::string email)\
        : email_(std::move(email))
    {
        auto& conn = connection();
        static bool prepared_finder = false;
        if (!prepared_finder)
        {
            conn.prepare("find_user_name",
                "SELECT user_name FROM users WHERE email = $1");
            prepared_finder = true;
        }
    }

    std::string UserNameFinder::FetchUserName() const
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_name"),
                pqxx::params{email_}
            );
            if (r.empty())
                return std::string{};
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            std::cerr << "UserNameFinder::FetchUserName failed: "
                      << e.what() << std::endl;
            return std::string{};
        }
    }

    IOSDeviceRegistrar::IOSDeviceRegistrar(std::string ios_device_id,
                                           std::string user_id,
                                           std::string apns_token,
                                           std::string device_name)
        : ios_device_id_(std::move(ios_device_id)),
          user_id_     (std::move(user_id)),
          apns_token_  (std::move(apns_token)),
          device_name_ (std::move(device_name))
    {
        auto& conn = connection();
        static bool prepared_IOSDevice = false;
        if (!prepared_IOSDevice)
        {
            conn.prepare(
                "register_ios_device",
                "INSERT INTO ios_devices "
                "(ios_device_id, user_id, apns_token, device_name, last_seen_at) "
                "VALUES (COALESCE(NULLIF($1,'')::uuid, gen_random_uuid()), $2, $3, $4, NOW()) "
                "ON CONFLICT (ios_device_id) DO UPDATE "
                "SET user_id      = EXCLUDED.user_id, "
                "    apns_token   = EXCLUDED.apns_token, "
                "    device_name  = EXCLUDED.device_name, "
                "    last_seen_at = NOW()");
            prepared_IOSDevice = true;
        }
    }

    bool IOSDeviceRegistrar::RegisterIOSDevice() const
    {
        try
        {
            pqxx::work tx{connection()};

            pqxx::params p;
            if (ios_device_id_.empty())
                p.append("");
            else
                p.append(ios_device_id_);
            p.append(user_id_);
            p.append(apns_token_);
            p.append(device_name_);

            tx.exec(pqxx::prepped("register_ios_device"), p);
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

    EdgeIOSBindingRegistrar::EdgeIOSBindingRegistrar(std::string edge_serial_number, std::string user_id)
        : edge_serial_number_(std::move(edge_serial_number)), user_id(std::move(user_id))
    {
        auto& conn = connection();
        static bool prepared_finder = false;
        if (!prepared_finder)
        {
            conn.prepare("bind_edge_user",
                "INSERT INTO edge_users "
                "(edge_serial_number, user_id) "
                "VALUES ($1, $2) "
                "ON CONFLICT DO NOTHING");

            conn.prepare("unbind_edge_user",
                "DELETE FROM edge_users "
                "WHERE edge_serial_number = $1 "
                "AND   user_id      = $2");

            prepared_finder = true;
        }
    }

    bool EdgeIOSBindingRegistrar::bind() const
    {
        try
        {
            pqxx::work tx{connection()};
            const auto r = tx.exec(
                pqxx::prepped("bind_edge_user"),
                pqxx::params{edge_serial_number_, user_id}
            );
            if (r.affected_rows() == 0)
                throw std::runtime_error("Binding already exists");
            tx.commit();
            return true;
        }
        catch ([[maybe_unused]] std::runtime_error& e)
        {
            throw;
        }
        catch (const std::exception& e)
        {
            std::cerr << "EdgeIOSBindingRegistrar::Bind failed: "
                      << e.what() << std::endl;
            return false;
        }
    }

    bool EdgeIOSBindingRegistrar::unbind() const
    {
        try
        {
            pqxx::work tx{connection()};
            tx.exec(
                pqxx::prepped("unbind_edge_user"),
                pqxx::params{edge_serial_number_, user_id}
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

    UserPasswordHashFinder::UserPasswordHashFinder(std::string email)
        : email_(std::move(email))
    {
        auto& conn = connection();
        static bool prepared = false;
        if (!prepared)
        {
            conn.prepare(
                "find_user_pwdhash",
                "SELECT user_password_hash "
                "FROM users "
                "WHERE email = $1");
            prepared = true;
        }
    }

    std::string UserPasswordHashFinder::FetchPasswordHash() const
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_pwdhash"),
                pqxx::params{email_}
            );
            if (r.empty())
                throw std::runtime_error("Email or Password Error");
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            std::cerr << "UserPasswordHashFinder::FetchPasswordHash failed: "
                      << e.what() << std::endl;
            throw;
        }
    }

    UserEdgeListFinder::UserEdgeListFinder(std::string user_id)
        : user_id_(std::move(user_id))
    {
        auto& conn = connection();
        static bool prepared = false;
        if (!prepared)
        {
            conn.prepare(
                "find_user_edges",
                "SELECT edge_serial_number "
                "FROM edge_users "
                "WHERE user_id = $1");
            prepared = true;
        }
    }

    std::vector<std::string> UserEdgeListFinder::FetchEdges() const
    {
        std::vector<std::string> edges;
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_edges"),
                pqxx::params{user_id_}
            );

            edges.reserve(r.size());
            for (auto row : r)
                edges.emplace_back(row[0].as<std::string>());
            return edges;                       // 空表也正常回 []
        }
        catch (const std::exception& e)
        {
            std::cerr << "UserEdgeListFinder::FetchEdges failed: "
                      << e.what() << std::endl;
            return edges;                       // 空表也正常回 []
        }
    }
}
