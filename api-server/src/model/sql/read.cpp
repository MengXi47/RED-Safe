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

#ifndef REDSAFE_FINDER_MODEL_CPP
#define REDSAFE_FINDER_MODEL_CPP

#include "read.hpp"
#include "../../util/IOstream.hpp"
#include "../../util/logger.hpp"

namespace redsafe::apiserver::model::sql::fin
{
    std::string UserIDFinder::start(std::string_view email)
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_id"),
                pqxx::params{email}
            );
            if (r.empty())
                return std::string{};
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            util::cerr() << "UserIDFinder::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "UserIDFinder::start failed: "
                    << e.what();
            return std::string{};
        }
    }

    std::string UserNameFinder::start(std::string_view email)
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_name"),
                pqxx::params{email}
            );
            if (r.empty())
                return std::string{};
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            util::cerr()
                    << "UserNameFinder::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "UserNameFinder::start failed: "
                    << e.what();
            return std::string{};
        }
    }

    std::string IOSDeviceIDFinder::start(std::string_view apns_token)
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_ios_device_id"),
                pqxx::params{apns_token}
            );
            if (r.empty())
                return std::string{};
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            util::cerr()
                    << "IOSDeviceFinder::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "IOSDeviceFinder::start failed: "
                    << e.what();
            return std::string{};
        }
    }

    std::string UserPasswordHashFinder::start(std::string_view email)
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_pwdhash"),
                pqxx::params{email}
            );
            if (r.empty())
                return std::string{};
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            util::cerr()
                    << "UserPasswordHashFinder::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "UserPasswordHashFinder::start failed: "
                    << e.what();
            return std::string{};
        }
    }

    std::vector<std::string> UserEdgeListFinder::start(std::string_view user_id)
    {
        std::vector<std::string> edges;
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("find_user_edges"),
                pqxx::params{user_id}
            );

            edges.reserve(r.size());
            for (auto row : r)
                edges.emplace_back(row[0].as<std::string>());
            return edges;
        }
        catch (const std::exception& e)
        {
            util::cerr()
                    << "UserEdgeListFinder::FetchEdges failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "UserEdgeListFinder::FetchEdges failed: "
                    << e.what();
            return edges;
        }
    }

    std::string RefreshTokenRefresher::start(std::string_view refresh_token_hash)
    {
        try
        {
            pqxx::work tx{connection()};
            const pqxx::result r = tx.exec(
                pqxx::prepped("chk_refretoken"),
                pqxx::params{refresh_token_hash}
            );
            if (r.empty())
            {
                tx.commit();
                return std::string{};
            }
            tx.commit();
            return r[0][0].as<std::string>();
        }
        catch (const std::exception& e)
        {
            util::cerr()
                    << "RefreshTokenRefresher::start failed: "
                    << e.what() << '\n';
            util::log(util::LogFile::server, util::Level::ERROR)
                    << "RefreshTokenRefresher::start failed: "
                    << e.what();
            return std::string{};
        }
    }
}

#endif