#pragma once

#include "db/db_error.h"
#include "db/db_result.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace db
{

    using Value = std::variant<std::nullptr_t, int64_t, double, std::string>;
    using Row = std::unordered_map<std::string, Value>;

    struct ExecResult
    {
        int64_t affectedRows{0};
        int64_t lastInsertId{0};
    };

    class IDatabase
    {
    public:
        virtual ~IDatabase() = default;

        // Connect / disconnect
        virtual Status connect() = 0;
        virtual void disconnect() = 0;

        // Exec: INSERT/UPDATE/DELETE/DDL
        virtual Result<ExecResult> exec(const std::string &sql) = 0;

        // Exec with positional parameters (SQLite uses '?' placeholders)
        virtual Result<ExecResult> exec(const std::string &sql, const std::vector<Value> &params) = 0;

        // Query: SELECT
        virtual Result<std::vector<Row>> query(const std::string &sql) = 0;

        // Query with positional parameters (SQLite uses '?' placeholders)
        virtual Result<std::vector<Row>> query(const std::string &sql, const std::vector<Value> &params) = 0;
    };

} // namespace db
