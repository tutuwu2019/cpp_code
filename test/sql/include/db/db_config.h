#pragma once

#include "db/db_types.h"

#include <cstddef>
#include <string>

namespace db
{

    struct SQLiteConfig
    {
        // ":memory:" or filesystem path
        std::string file{":memory:"};

        // Enable WAL journaling mode (recommended for concurrency).
        // Note: for in-memory DB, WAL is not applicable.
        bool walEnabled{false};

        // When WAL is enabled, it's common to set synchronous=NORMAL for better performance.
        bool synchronousNormal{true};
    };

    struct MySQLConfig
    {
        std::string host{"127.0.0.1"};
        uint16_t port{3306};
        std::string user{"root"};
        std::string password;
        std::string database;

        // Connection pool
        std::size_t maxSize{10};
    };

    struct DBConfig
    {
        DBType type{DBType::SQLite};
        SQLiteConfig sqlite;
        MySQLConfig mysql;
    };

} // namespace db
