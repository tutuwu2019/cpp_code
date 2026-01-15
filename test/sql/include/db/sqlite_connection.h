#pragma once

#include "db/db_config.h"
#include "db/i_database.h"

struct sqlite3;

namespace db
{

    class SQLiteConnection final : public IDatabase
    {
    public:
        explicit SQLiteConnection(SQLiteConfig cfg);
        ~SQLiteConnection() override;

        Status connect() override;
        void disconnect() override;

        Result<ExecResult> exec(const std::string &sql) override;
        Result<ExecResult> exec(const std::string &sql, const std::vector<Value> &params) override;
        Result<std::vector<Row>> query(const std::string &sql) override;
        Result<std::vector<Row>> query(const std::string &sql, const std::vector<Value> &params) override;

    private:
        SQLiteConfig cfg_;
        sqlite3 *db_{nullptr};
    };

} // namespace db
