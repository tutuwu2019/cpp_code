#include "db/sqlite_connection.h"

#if defined(DB_HAS_SQLITE3)

#include <sqlite3.h>

namespace db
{

    static Status bind_params(sqlite3_stmt *stmt, const std::vector<Value> &params)
    {
        for (std::size_t i = 0; i < params.size(); ++i)
        {
            int idx = static_cast<int>(i + 1); // sqlite bind index starts at 1
            const auto &v = params[i];

            int rc = SQLITE_OK;
            if (std::holds_alternative<std::nullptr_t>(v))
            {
                rc = sqlite3_bind_null(stmt, idx);
            }
            else if (std::holds_alternative<int64_t>(v))
            {
                rc = sqlite3_bind_int64(stmt, idx, std::get<int64_t>(v));
            }
            else if (std::holds_alternative<double>(v))
            {
                rc = sqlite3_bind_double(stmt, idx, std::get<double>(v));
            }
            else if (std::holds_alternative<std::string>(v))
            {
                const auto &s = std::get<std::string>(v);
                rc = sqlite3_bind_text(stmt, idx, s.c_str(), static_cast<int>(s.size()), SQLITE_TRANSIENT);
            }
            else
            {
                rc = SQLITE_MISUSE;
            }

            if (rc != SQLITE_OK)
            {
                return Status::driver_error(std::string("sqlite bind param failed: ") + sqlite3_errstr(rc));
            }
        }
        return Status::success();
    }

    static Status to_status(int rc, sqlite3 *db, const char *prefix)
    {
        if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
        {
            return Status::success();
        }
        const char *msg = db ? sqlite3_errmsg(db) : "unknown sqlite error";
        return Status::driver_error(std::string(prefix) + ": " + msg);
    }

    SQLiteConnection::SQLiteConnection(SQLiteConfig cfg) : cfg_(std::move(cfg)) {}

    SQLiteConnection::~SQLiteConnection() { disconnect(); }

    Status SQLiteConnection::connect()
    {
        if (db_)
            return Status::success();
        int rc = sqlite3_open(cfg_.file.c_str(), &db_);
        if (rc != SQLITE_OK)
        {
            Status s = to_status(rc, db_, "sqlite3_open");
            disconnect();
            return s;
        }

        // WAL configuration: can be forced at compile-time via DB_SQLITE_WAL
        bool wantWal = cfg_.walEnabled;
#if defined(DB_SQLITE_WAL)
        wantWal = true;
#endif
        if (wantWal && cfg_.file != ":memory:")
        {
            // journal_mode=WAL
            char *errMsg = nullptr;
            int prc = sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, &errMsg);
            if (prc != SQLITE_OK)
            {
                std::string msg = errMsg ? errMsg : "PRAGMA journal_mode=WAL failed";
                sqlite3_free(errMsg);
                return Status::driver_error(std::move(msg));
            }
            sqlite3_free(errMsg);

            if (cfg_.synchronousNormal)
            {
                errMsg = nullptr;
                prc = sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, &errMsg);
                if (prc != SQLITE_OK)
                {
                    std::string msg = errMsg ? errMsg : "PRAGMA synchronous=NORMAL failed";
                    sqlite3_free(errMsg);
                    return Status::driver_error(std::move(msg));
                }
                sqlite3_free(errMsg);
            }
        }
        return Status::success();
    }

    void SQLiteConnection::disconnect()
    {
        if (db_)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    Result<ExecResult> SQLiteConnection::exec(const std::string &sql)
    {
        if (!db_)
        {
            auto s = connect();
            if (!s.ok())
                return Result<ExecResult>::error(std::move(s));
        }

        return exec(sql, {});
    }

    Result<ExecResult> SQLiteConnection::exec(const std::string &sql, const std::vector<Value> &params)
    {
        if (!db_)
        {
            auto s = connect();
            if (!s.ok())
                return Result<ExecResult>::error(std::move(s));
        }

        sqlite3_stmt *stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            return Result<ExecResult>::error(to_status(rc, db_, "sqlite3_prepare_v2"));
        }

        auto bs = bind_params(stmt, params);
        if (!bs.ok())
        {
            sqlite3_finalize(stmt);
            return Result<ExecResult>::error(std::move(bs));
        }

        // Step until done (for safety with multi-row statements)
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            // ignore rows
        }
        if (rc != SQLITE_DONE)
        {
            Status s = to_status(rc, db_, "sqlite3_step");
            sqlite3_finalize(stmt);
            return Result<ExecResult>::error(std::move(s));
        }

        sqlite3_finalize(stmt);

        ExecResult r;
        r.affectedRows = sqlite3_changes(db_);
        r.lastInsertId = sqlite3_last_insert_rowid(db_);
        return Result<ExecResult>::ok(r);
    }

    Result<std::vector<Row>> SQLiteConnection::query(const std::string &sql)
    {
        if (!db_)
        {
            auto s = connect();
            if (!s.ok())
                return Result<std::vector<Row>>::error(std::move(s));
        }

        return query(sql, {});
    }

    Result<std::vector<Row>> SQLiteConnection::query(const std::string &sql, const std::vector<Value> &params)
    {
        if (!db_)
        {
            auto s = connect();
            if (!s.ok())
                return Result<std::vector<Row>>::error(std::move(s));
        }

        sqlite3_stmt *stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            return Result<std::vector<Row>>::error(to_status(rc, db_, "sqlite3_prepare_v2"));
        }

        auto bs = bind_params(stmt, params);
        if (!bs.ok())
        {
            sqlite3_finalize(stmt);
            return Result<std::vector<Row>>::error(std::move(bs));
        }

        std::vector<Row> rows;

        int colCount = sqlite3_column_count(stmt);
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            Row row;
            for (int i = 0; i < colCount; ++i)
            {
                const char *name = sqlite3_column_name(stmt, i);
                int type = sqlite3_column_type(stmt, i);
                switch (type)
                {
                case SQLITE_INTEGER:
                    row[name] = static_cast<int64_t>(sqlite3_column_int64(stmt, i));
                    break;
                case SQLITE_FLOAT:
                    row[name] = sqlite3_column_double(stmt, i);
                    break;
                case SQLITE_TEXT:
                    row[name] = std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, i)));
                    break;
                case SQLITE_NULL:
                default:
                    row[name] = nullptr;
                    break;
                }
            }
            rows.emplace_back(std::move(row));
        }

        sqlite3_finalize(stmt);

        if (rc != SQLITE_DONE)
        {
            return Result<std::vector<Row>>::error(to_status(rc, db_, "sqlite3_step"));
        }

        return Result<std::vector<Row>>::ok(std::move(rows));
    }

} // namespace db

#else

namespace db
{

    SQLiteConnection::SQLiteConnection(SQLiteConfig cfg) : cfg_(std::move(cfg)) {}
    SQLiteConnection::~SQLiteConnection() { disconnect(); }

    Status SQLiteConnection::connect()
    {
        return Status::not_implemented("SQLite3 headers not found; rebuild with sqlite3 dev package");
    }
    void SQLiteConnection::disconnect() { db_ = nullptr; }

    Result<ExecResult> SQLiteConnection::exec(const std::string &)
    {
        return Result<ExecResult>::error(Status::not_implemented("SQLiteConnection::exec"));
    }

    Result<ExecResult> SQLiteConnection::exec(const std::string &, const std::vector<Value> &)
    {
        return Result<ExecResult>::error(Status::not_implemented("SQLiteConnection::exec(params)"));
    }

    Result<std::vector<Row>> SQLiteConnection::query(const std::string &)
    {
        return Result<std::vector<Row>>::error(Status::not_implemented("SQLiteConnection::query"));
    }

    Result<std::vector<Row>> SQLiteConnection::query(const std::string &, const std::vector<Value> &)
    {
        return Result<std::vector<Row>>::error(Status::not_implemented("SQLiteConnection::query(params)"));
    }

} // namespace db

#endif
