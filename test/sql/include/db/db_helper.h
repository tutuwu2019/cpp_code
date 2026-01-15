#pragma once

#include "db/db_config.h"
#include "db/db_result.h"
#include "db/i_database.h"
#include "db/mysql_connection.h"
#include "db/mysql_connection_pool.h"
#include "db/sqlite_connection.h"

#include <memory>
#include <optional>

namespace db
{

    // 按Diagram：Client -> DBHelper -> (SQLiteAdapter/MySQLAdapter) -> (Connection/Pool)
    class DBHelper
    {
    public:
        explicit DBHelper(DBConfig cfg);

        // Generic DB operations
        Result<ExecResult> exec(const std::string &sql);
        Result<ExecResult> exec(const std::string &sql, const std::vector<Value> &params);
        Result<std::vector<Row>> query(const std::string &sql);
        Result<std::vector<Row>> query(const std::string &sql, const std::vector<Value> &params);

        // Minimal CRUD for `user` table demonstrating flow.
        Result<int64_t> createUser(const std::string &username,
                                   const std::string &password_hash,
                                   const std::optional<std::string> &email = std::nullopt);

    private:
        DBConfig cfg_;

        // SQLite path
        std::unique_ptr<SQLiteConnection> sqlite_;

        // MySQL path
        std::unique_ptr<MySQLConnectionPool> mysqlPool_;
    };

} // namespace db
