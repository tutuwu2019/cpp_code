#include "db/db_helper.h"

#include <sstream>

namespace db
{

    namespace
    {
        class PooledMySQLConn
        {
        public:
            PooledMySQLConn(MySQLConnectionPool &pool, std::shared_ptr<MySQLConnection> conn)
                : pool_(pool), conn_(std::move(conn))
            {
            }
            ~PooledMySQLConn()
            {
                if (conn_)
                    pool_.release(std::move(conn_));
            }

            MySQLConnection *operator->() const { return conn_.get(); }
            explicit operator bool() const { return static_cast<bool>(conn_); }

        private:
            MySQLConnectionPool &pool_;
            std::shared_ptr<MySQLConnection> conn_;
        };
    }

    static std::string sqlEscapeSingleQuotes(const std::string &s)
    {
        // minimal escape for demo: ' -> ''
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            if (c == '\'')
                out += "''";
            else
                out.push_back(c);
        }
        return out;
    }

    DBHelper::DBHelper(DBConfig cfg) : cfg_(std::move(cfg))
    {
        if (cfg_.type == DBType::SQLite)
        {
            sqlite_ = std::make_unique<SQLiteConnection>(cfg_.sqlite);
        }
        else
        {
            mysqlPool_ = std::make_unique<MySQLConnectionPool>(cfg_.mysql);
        }
    }

    Result<ExecResult> DBHelper::exec(const std::string &sql)
    {
        if (cfg_.type == DBType::SQLite)
        {
            return sqlite_->exec(sql);
        }

        auto acquired = mysqlPool_->acquire(std::chrono::milliseconds(200));
        if (!acquired.ok())
            return Result<ExecResult>::error(std::move(acquired.status));

        PooledMySQLConn conn(*mysqlPool_, acquired.value);
        return conn->exec(sql);
    }

    Result<ExecResult> DBHelper::exec(const std::string &sql, const std::vector<Value> &params)
    {
        if (cfg_.type == DBType::SQLite)
        {
            return sqlite_->exec(sql, params);
        }

        auto acquired = mysqlPool_->acquire(std::chrono::milliseconds(200));
        if (!acquired.ok())
            return Result<ExecResult>::error(std::move(acquired.status));

        PooledMySQLConn conn(*mysqlPool_, acquired.value);
        return conn->exec(sql, params);
    }

    Result<std::vector<Row>> DBHelper::query(const std::string &sql)
    {
        if (cfg_.type == DBType::SQLite)
        {
            return sqlite_->query(sql);
        }

        auto acquired = mysqlPool_->acquire(std::chrono::milliseconds(200));
        if (!acquired.ok())
            return Result<std::vector<Row>>::error(std::move(acquired.status));

        PooledMySQLConn conn(*mysqlPool_, acquired.value);
        return conn->query(sql);
    }

    Result<std::vector<Row>> DBHelper::query(const std::string &sql, const std::vector<Value> &params)
    {
        if (cfg_.type == DBType::SQLite)
        {
            return sqlite_->query(sql, params);
        }

        auto acquired = mysqlPool_->acquire(std::chrono::milliseconds(200));
        if (!acquired.ok())
            return Result<std::vector<Row>>::error(std::move(acquired.status));

        PooledMySQLConn conn(*mysqlPool_, acquired.value);
        return conn->query(sql, params);
    }

    Result<int64_t> DBHelper::createUser(const std::string &username,
                                         const std::string &password_hash,
                                         const std::optional<std::string> &email)
    {
        if (username.empty() || password_hash.empty())
        {
            return Result<int64_t>::error(Status::invalid_argument("username/password_hash empty"));
        }

        std::ostringstream oss;
        oss << "INSERT INTO user(username, password_hash";
        if (email.has_value())
            oss << ", email";
        oss << ") VALUES('" << sqlEscapeSingleQuotes(username) << "', '" << sqlEscapeSingleQuotes(password_hash) << "'";
        if (email.has_value())
            oss << ", '" << sqlEscapeSingleQuotes(*email) << "'";
        oss << ");";

        const std::string sql = oss.str();

        if (cfg_.type == DBType::SQLite)
        {
            auto r = sqlite_->exec(sql);
            if (!r.ok())
                return Result<int64_t>::error(std::move(r.status));
            return Result<int64_t>::ok(r.value.lastInsertId);
        }

        // MySQL path (shape according to Diagram)
        auto acquired = mysqlPool_->acquire(std::chrono::milliseconds(200));
        if (!acquired.ok())
            return Result<int64_t>::error(std::move(acquired.status));

        PooledMySQLConn conn(*mysqlPool_, acquired.value);
        auto r = conn->exec(sql);

        if (!r.ok())
            return Result<int64_t>::error(std::move(r.status));
        return Result<int64_t>::ok(r.value.lastInsertId);
    }

} // namespace db
