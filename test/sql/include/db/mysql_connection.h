#pragma once

#include "db/db_config.h"
#include "db/i_database.h"

namespace db
{

    // 说明：这里提供“形状正确”的连接对象与接口；真实MySQL驱动（mysqlclient / mysql-connector-c++）未接入时会返回 NotImplemented。
    class MySQLConnection final : public IDatabase
    {
    public:
        explicit MySQLConnection(MySQLConfig cfg);
        ~MySQLConnection() override;

        Status connect() override;
        void disconnect() override;

        Result<ExecResult> exec(const std::string &sql) override;
        Result<ExecResult> exec(const std::string &sql, const std::vector<Value> &params) override;
        Result<std::vector<Row>> query(const std::string &sql) override;
        Result<std::vector<Row>> query(const std::string &sql, const std::vector<Value> &params) override;

    private:
        MySQLConfig cfg_;
        bool connected_{false};

        struct Impl;
        Impl *impl_{nullptr};
    };

} // namespace db
