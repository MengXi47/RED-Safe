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
    // 創建Table
    class TableCreator : public ConnectionManager
    {
    public:
        explicit TableCreator(std::string table_name);
        [[nodiscard]] bool CreateTable() const;
    private:
        std::string table_name_;
    };

    // 刪除Table
    class TableDeleter : public ConnectionManager
    {
    public:
        explicit TableDeleter(std::string  table_name);
        [[nodiscard]] bool DropTable() const;
    private:
        std::string table_name_;
    };

    // 欄位是否存在 💩
    class TableQuerier : public ConnectionManager
    {
    public:
        explicit TableQuerier(std::string  table_name);
        [[nodiscard]] bool ColumnExists(const std::string& column_name) const;
    private:
        std::string table_name_;
    };
    
    // 對欄位賦值 (沒有則建立)
    class TableColumnAssigner : public ConnectionManager
    {
    public:
        explicit TableColumnAssigner(std::string  table_name);
        [[nodiscard]] bool SetColumnValue(const std::string& column_name,
                            const std::string& column_type,
                            const std::string& value) const;
    private:
        std::string table_name_;
    };

    // 檢查Table是否存在
    class TableExists : public ConnectionManager
    {
    public:
        explicit TableExists(std::string  table_name);
        [[nodiscard]] bool Exists() const;
    private:
        std::string table_name_;
    };

    // 新增欄位 (若不存在則新增)
    class ColumnAdder : public ConnectionManager
    {
    public:
        explicit ColumnAdder(std::string  table_name);
        [[nodiscard]] bool AddColumn(const std::string& column_name,
                       const std::string& column_type) const;
    private:
        std::string table_name_;
    };

    // 刪除欄位 (若存在則刪除)
    class ColumnRemover : public ConnectionManager
    {
    public:
        explicit ColumnRemover(std::string  table_name);
        [[nodiscard]] bool RemoveColumn(const std::string& column_name) const;
    private:
        std::string table_name_;
    };

    // 查詢欄位的所有值並回傳
    class ColumnValueFetcher : public ConnectionManager
    {
    public:
        explicit ColumnValueFetcher(std::string  table_name);
        [[nodiscard]] std::string FetchValue(const std::string& column_name) const;
    private:
        std::string table_name_;
    };
}