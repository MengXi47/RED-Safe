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
#include <vector>

namespace redsafe::apiserver::model::sql
{
    TableCreator::TableCreator(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    bool TableCreator::CreateTable()
    {
        try
        {
            pqxx::work W{connection()};
            std::string quotedName = W.quote_name(table_name_);
            std::string createSql = "CREATE TABLE IF NOT EXISTS " + quotedName +
                " ()";
            auto R1 = W.exec(createSql);
            W.commit();
            return false;
        }
        catch (const pqxx::sql_error &e)
        {
            std::cerr << "SQL error: " << e.what()
                      << "\nDuring query: " << e.query() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Unexpected error: " << e.what() << std::endl;
        }
        return true;
    }

    TableDeleter::TableDeleter(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    bool TableDeleter::DropTable()
    {
        try
        {
            pqxx::work W{connection()};
            std::string quotedName = W.quote_name(table_name_);
            std::string dropSql = "DROP TABLE IF EXISTS " + quotedName + ";";
            W.exec(dropSql);
            W.commit();
            return false;
        }
        catch (const pqxx::sql_error &e)
        {
            std::cerr << "SQL error during drop: " << e.what()
                      << "\nDuring query: " << e.query() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Unexpected error during drop: " << e.what() << std::endl;
        }
        return true;
    }

    TableQuerier::TableQuerier(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    bool TableQuerier::ColumnExists(const std::string &column_name)
    {
        pqxx::work W{connection()};
        pqxx::params params;
        params.append(table_name_);
        params.append(column_name);
        auto R = W.exec(
            "SELECT 1 FROM information_schema.columns "
            "WHERE table_schema = 'public' "
            "  AND table_name   = $1 "
            "  AND column_name  = $2 "
            "LIMIT 1;",
            params);
        return !R.empty();
    }


    TableColumnAssigner::TableColumnAssigner(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    bool TableColumnAssigner::SetColumnValue(const std::string &column_name,
                                             const std::string &column_type,
                                             const std::string &value)
    {
        try
        {
            pqxx::work W{connection()};
            std::string tableQuoted = W.quote_name(table_name_);
            std::string colQuoted   = W.quote_name(column_name);
            std::string alterSql = "ALTER TABLE " + tableQuoted +
                " ADD COLUMN IF NOT EXISTS " + colQuoted + " " + column_type + ";";
            W.exec(alterSql);
            std::string valQuoted = W.quote(value);
            std::string updateSql = "UPDATE " + tableQuoted +
                " SET " + colQuoted + " = " + valQuoted + ";";
            W.exec(updateSql);
            W.commit();
            std::cout << "Column '" << column_name << "' ensured and set on table '"
                      << table_name_ << "'." << std::endl;
            return false;
        }
        catch (const pqxx::sql_error &e)
        {
            std::cerr << "SQL error: " << e.what()
                      << "\nDuring query: " << e.query() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        return true;
    }

    TableExists::TableExists(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    bool TableExists::Exists()
    {
        try
        {
            pqxx::work W{connection()};
            pqxx::params params;
            params.append(table_name_);
            auto R = W.exec(
                "SELECT 1 FROM information_schema.tables "
                "WHERE table_schema = 'public' AND table_name = $1 LIMIT 1;",
                params);
            return !R.empty();
        }
        catch (const pqxx::sql_error &e)
        {
            std::cerr << "SQL error in TableExists::Exists: " << e.what()
                      << "\nDuring query: " << e.query() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in TableExists::Exists: " << e.what() << std::endl;
        }
        return false;
    }

    ColumnAdder::ColumnAdder(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    // Add a column if it does not exist
    bool ColumnAdder::AddColumn(const std::string &column_name,
                                const std::string &column_type)
    {
        try
        {
            pqxx::work W{connection()};
            std::string tableQuoted = W.quote_name(table_name_);
            std::string colQuoted   = W.quote_name(column_name);
            std::string alterSql = "ALTER TABLE " + tableQuoted +
                " ADD COLUMN IF NOT EXISTS " + colQuoted +
                " " + column_type + ";";
            W.exec(alterSql);
            W.commit();
            return true;
        }
        catch (const pqxx::sql_error &e)
        {
            std::cerr << "SQL error in ColumnAdder::AddColumn: " << e.what()
                      << "\nDuring query: " << e.query() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in ColumnAdder::AddColumn: " << e.what() << std::endl;
        }
        return false;
    }

    ColumnRemover::ColumnRemover(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    bool ColumnRemover::RemoveColumn(const std::string &column_name)
    {
        try
        {
            pqxx::work W{connection()};
            std::string tableQuoted = W.quote_name(table_name_);
            std::string colQuoted   = W.quote_name(column_name);
            std::string alterSql = "ALTER TABLE " + tableQuoted +
                " DROP COLUMN IF EXISTS " + colQuoted + ";";
            W.exec(alterSql);
            W.commit();
            return true;
        }
        catch (const pqxx::sql_error &e)
        {
            std::cerr << "SQL error in ColumnRemover::RemoveColumn: " << e.what()
                      << "\nDuring query: " << e.query() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in ColumnRemover::RemoveColumn: " << e.what() << std::endl;
        }
        return false;
    }

    ColumnValueFetcher::ColumnValueFetcher(const std::string &table_name)
        : ConnectionManager(), table_name_(table_name)
    {
    }

    std::string ColumnValueFetcher::FetchValue(const std::string &column_name)
    {
        try
        {
            pqxx::work W{connection()};
            std::string tableQuoted = W.quote_name(table_name_);
            std::string colQuoted   = W.quote_name(column_name);
            std::string selectSql = "SELECT " + colQuoted +
                " FROM " + tableQuoted +
                " LIMIT 1;";
            auto R = W.exec(selectSql);
            if (!R.empty())
                return R[0][colQuoted].c_str();
        }
        catch (const pqxx::sql_error &e)
        {
            std::cerr << "SQL error in ColumnValueFetcher::FetchValues: " << e.what()
                      << "\nDuring query: " << e.query() << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in ColumnValueFetcher::FetchValues: " << e.what() << std::endl;
        }
        return "";
    }
}