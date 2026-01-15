#include "db/mysql_connection.h"

#if defined(DB_HAS_MYSQLCLIENT)

#include <mysql/mysql.h>

#include <cstring>
#include <algorithm>
#include <memory>

namespace db
{
    // mysqlclient 8 removed the public my_bool typedef, but MYSQL_BIND::is_null
    // is still a boolean flag pointer. Use a local alias for portability.
    // IMPORTANT: don't use bool or std::vector<bool> here, because MySQL C API
    // expects a real pointer to an addressable byte.
    using mysql_bool_t = unsigned char;
    namespace
    {
        struct MySQLHandle
        {
            MYSQL *mysql{nullptr};
            MySQLHandle() : mysql(mysql_init(nullptr)) {}
            ~MySQLHandle()
            {
                if (mysql)
                {
                    mysql_close(mysql);
                    mysql = nullptr;
                }
            }
        };

        Status mysql_status(MYSQL *m, const char *prefix)
        {
            if (!m)
                return Status::driver_error(std::string(prefix) + ": null mysql handle");
            return Status::driver_error(std::string(prefix) + ": " + mysql_error(m));
        }

        Status stmt_status(MYSQL_STMT *s, const char *prefix)
        {
            if (!s)
                return Status::driver_error(std::string(prefix) + ": null stmt");
            return Status::driver_error(std::string(prefix) + ": " + mysql_stmt_error(s));
        }

        // helper: bind params to MYSQL_BIND
        struct BoundParamBuffers
        {
            // store owned buffers for string/number so pointers remain valid during execute
            std::vector<long long> i64;
            std::vector<double> f64;
            // index-aligned string storage to keep c_str pointers stable
            std::vector<std::string> str;
            std::vector<unsigned long> strLen;
            std::vector<mysql_bool_t> isNull;
        };

        Status bind_params(MYSQL_STMT *stmt, const std::vector<Value> &params)
        {
            if (params.empty())
                return Status::success();

            std::vector<MYSQL_BIND> binds(params.size());
            BoundParamBuffers bufs;
            bufs.isNull.resize(params.size());

            // IMPORTANT: pointers stored in MYSQL_BIND must remain valid until mysql_stmt_execute
            // completes. Using push_back() and then taking &vector.back() will break once the
            // vector reallocates. So we pre-size numeric buffers and assign by index.
            bufs.i64.assign(params.size(), 0);
            bufs.f64.assign(params.size(), 0.0);
            bufs.str.assign(params.size(), std::string{});
            bufs.strLen.assign(params.size(), 0);

            for (std::size_t i = 0; i < params.size(); ++i)
            {
                MYSQL_BIND &b = binds[i];
                std::memset(&b, 0, sizeof(b));
                const auto &v = params[i];

                if (std::holds_alternative<std::nullptr_t>(v))
                {
                    b.buffer_type = MYSQL_TYPE_NULL;
                    bufs.isNull[i] = 1;
                    b.is_null = reinterpret_cast<bool *>(&bufs.isNull[i]);
                }
                else if (std::holds_alternative<int64_t>(v))
                {
                    bufs.i64[i] = static_cast<long long>(std::get<int64_t>(v));
                    b.buffer_type = MYSQL_TYPE_LONGLONG;
                    b.buffer = &bufs.i64[i];
                    bufs.isNull[i] = 0;
                    b.is_null = reinterpret_cast<bool *>(&bufs.isNull[i]);
                }
                else if (std::holds_alternative<double>(v))
                {
                    bufs.f64[i] = std::get<double>(v);
                    b.buffer_type = MYSQL_TYPE_DOUBLE;
                    b.buffer = &bufs.f64[i];
                    bufs.isNull[i] = 0;
                    b.is_null = reinterpret_cast<bool *>(&bufs.isNull[i]);
                }
                else if (std::holds_alternative<std::string>(v))
                {
                    bufs.str[i] = std::get<std::string>(v);
                    bufs.strLen[i] = static_cast<unsigned long>(bufs.str[i].size());
                    b.buffer_type = MYSQL_TYPE_STRING;
                    b.buffer = const_cast<char *>(bufs.str[i].c_str());
                    b.buffer_length = bufs.strLen[i];
                    b.length = &bufs.strLen[i];
                    bufs.isNull[i] = 0;
                    b.is_null = reinterpret_cast<bool *>(&bufs.isNull[i]);
                }
                else
                {
                    return Status::invalid_argument("unsupported param type");
                }
            }

            // Important: bufs must stay alive until execute finishes.
            // We can't return bufs without changing signature; instead, do bind+execute in one scope.
            // Caller will re-create buffers per execute.
            if (mysql_stmt_bind_param(stmt, binds.data()) != 0)
            {
                return stmt_status(stmt, "mysql_stmt_bind_param");
            }
            if (mysql_stmt_execute(stmt) != 0)
            {
                return stmt_status(stmt, "mysql_stmt_execute");
            }
            return Status::success();
        }
    } // namespace

    struct MySQLConnection::Impl
    {
        MySQLHandle handle;
        bool connected{false};
    };

    MySQLConnection::MySQLConnection(MySQLConfig cfg) : cfg_(std::move(cfg)) {}
    MySQLConnection::~MySQLConnection()
    {
        disconnect();
        delete impl_;
        impl_ = nullptr;
    }

    Status MySQLConnection::connect()
    {
        if (!impl_)
            impl_ = new Impl();

        auto &p = *impl_;
        if (!p.handle.mysql)
            return Status::driver_error("mysql_init failed");

        if (p.connected)
            return Status::success();

        // Enable reconnect
        // MySQL 8 headers may not expose my_bool; MYSQL_OPT_RECONNECT expects a bool-ish pointer.
        bool reconnect = true;
        mysql_options(p.handle.mysql, MYSQL_OPT_RECONNECT, &reconnect);

        MYSQL *ret = mysql_real_connect(p.handle.mysql,
                                        cfg_.host.c_str(),
                                        cfg_.user.c_str(),
                                        cfg_.password.c_str(),
                                        cfg_.database.empty() ? nullptr : cfg_.database.c_str(),
                                        cfg_.port,
                                        nullptr,
                                        0);
        if (!ret)
        {
            return mysql_status(p.handle.mysql, "mysql_real_connect");
        }

        p.connected = true;
        connected_ = true;
        return Status::success();
    }

    void MySQLConnection::disconnect()
    {
        connected_ = false;
        if (impl_)
        {
            impl_->connected = false;
        }
    }

    Result<ExecResult> MySQLConnection::exec(const std::string &sql)
    {
        return exec(sql, {});
    }

    Result<ExecResult> MySQLConnection::exec(const std::string &sql, const std::vector<Value> &params)
    {
        auto s = connect();
        if (!s.ok())
            return Result<ExecResult>::error(std::move(s));

        auto &p = *impl_;

        // If no params, run direct query
        if (params.empty())
        {
            if (mysql_real_query(p.handle.mysql, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
            {
                return Result<ExecResult>::error(mysql_status(p.handle.mysql, "mysql_real_query"));
            }
            ExecResult r;
            r.affectedRows = static_cast<int64_t>(mysql_affected_rows(p.handle.mysql));
            r.lastInsertId = static_cast<int64_t>(mysql_insert_id(p.handle.mysql));
            return Result<ExecResult>::ok(r);
        }

        MYSQL_STMT *stmt = mysql_stmt_init(p.handle.mysql);
        if (!stmt)
            return Result<ExecResult>::error(Status::driver_error("mysql_stmt_init failed"));

        if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
        {
            auto st = stmt_status(stmt, "mysql_stmt_prepare");
            mysql_stmt_close(stmt);
            return Result<ExecResult>::error(std::move(st));
        }

        auto st = bind_params(stmt, params);
        if (!st.ok())
        {
            mysql_stmt_close(stmt);
            return Result<ExecResult>::error(std::move(st));
        }

        ExecResult r;
        r.affectedRows = static_cast<int64_t>(mysql_stmt_affected_rows(stmt));
        r.lastInsertId = static_cast<int64_t>(mysql_stmt_insert_id(stmt));

        mysql_stmt_close(stmt);
        return Result<ExecResult>::ok(r);
    }

    Result<std::vector<Row>> MySQLConnection::query(const std::string &sql)
    {
        return query(sql, {});
    }

    Result<std::vector<Row>> MySQLConnection::query(const std::string &sql, const std::vector<Value> &params)
    {
        auto s = connect();
        if (!s.ok())
            return Result<std::vector<Row>>::error(std::move(s));

        auto &p = *impl_;

        // NOTE: For simplicity, we use text protocol for SELECT without params,
        // and prepared stmt only for param queries.
        if (params.empty())
        {
            if (mysql_real_query(p.handle.mysql, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
            {
                return Result<std::vector<Row>>::error(mysql_status(p.handle.mysql, "mysql_real_query"));
            }

            MYSQL_RES *res = mysql_store_result(p.handle.mysql);
            if (!res)
            {
                // no result-set (maybe query not returning rows)
                return Result<std::vector<Row>>::ok({});
            }

            int numFields = mysql_num_fields(res);
            MYSQL_FIELD *fields = mysql_fetch_fields(res);

            std::vector<Row> out;
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                unsigned long *lengths = mysql_fetch_lengths(res);
                Row r;
                for (int i = 0; i < numFields; ++i)
                {
                    const char *name = fields[i].name;
                    if (!row[i])
                    {
                        r[name] = nullptr;
                        continue;
                    }
                    // conservative: store as string
                    r[name] = std::string(row[i], lengths[i]);
                }
                out.emplace_back(std::move(r));
            }
            mysql_free_result(res);
            return Result<std::vector<Row>>::ok(std::move(out));
        }

        MYSQL_STMT *stmt = mysql_stmt_init(p.handle.mysql);
        if (!stmt)
            return Result<std::vector<Row>>::error(Status::driver_error("mysql_stmt_init failed"));

        if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
        {
            auto st = stmt_status(stmt, "mysql_stmt_prepare");
            mysql_stmt_close(stmt);
            return Result<std::vector<Row>>::error(std::move(st));
        }

        // Bind + execute
        auto st = bind_params(stmt, params);
        if (!st.ok())
        {
            mysql_stmt_close(stmt);
            return Result<std::vector<Row>>::error(std::move(st));
        }

        // Retrieve metadata
        MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
        if (!meta)
        {
            mysql_stmt_close(stmt);
            return Result<std::vector<Row>>::ok({});
        }
        int numFields = mysql_num_fields(meta);
        MYSQL_FIELD *fields = mysql_fetch_fields(meta);

        // Bind result buffers (as strings)
        std::vector<MYSQL_BIND> rb(numFields);
        std::vector<std::vector<char>> buffers(numFields);
        std::vector<unsigned long> lengths(numFields);
        std::vector<mysql_bool_t> isNull(numFields);

        for (int i = 0; i < numFields; ++i)
        {
            std::memset(&rb[i], 0, sizeof(MYSQL_BIND));
            // allocate a reasonable buffer; if truncated, we can re-fetch by mysql_stmt_fetch_column
            unsigned long cap = 4096;
            buffers[i].resize(cap);
            rb[i].buffer_type = MYSQL_TYPE_STRING;
            rb[i].buffer = buffers[i].data();
            rb[i].buffer_length = cap;
            rb[i].length = &lengths[i];
            rb[i].is_null = reinterpret_cast<bool *>(&isNull[i]);
        }

        if (mysql_stmt_bind_result(stmt, rb.data()) != 0)
        {
            auto st2 = stmt_status(stmt, "mysql_stmt_bind_result");
            mysql_free_result(meta);
            mysql_stmt_close(stmt);
            return Result<std::vector<Row>>::error(std::move(st2));
        }

        if (mysql_stmt_store_result(stmt) != 0)
        {
            auto st2 = stmt_status(stmt, "mysql_stmt_store_result");
            mysql_free_result(meta);
            mysql_stmt_close(stmt);
            return Result<std::vector<Row>>::error(std::move(st2));
        }

        std::vector<Row> out;
        int fetchRc = 0;
        while ((fetchRc = mysql_stmt_fetch(stmt)) == 0 || fetchRc == MYSQL_DATA_TRUNCATED)
        {
            Row r;
            for (int i = 0; i < numFields; ++i)
            {
                const char *name = fields[i].name;
                if (isNull[i] != 0)
                {
                    r[name] = nullptr;
                }
                else
                {
                    // If truncated, lengths[i] > buffer_length; keep only what we have.
                    r[name] = std::string(buffers[i].data(), std::min<unsigned long>(lengths[i], rb[i].buffer_length));
                }
            }
            out.emplace_back(std::move(r));
        }

        mysql_free_result(meta);
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
        return Result<std::vector<Row>>::ok(std::move(out));
    }

} // namespace db

#else

namespace db
{

    MySQLConnection::MySQLConnection(MySQLConfig cfg) : cfg_(std::move(cfg)) {}
    MySQLConnection::~MySQLConnection() { disconnect(); }

    Status MySQLConnection::connect()
    {
        connected_ = false;
        return Status::not_implemented("MySQLConnection: mysqlclient not integrated");
    }

    void MySQLConnection::disconnect() { connected_ = false; }

    Result<ExecResult> MySQLConnection::exec(const std::string &) { return Result<ExecResult>::error(Status::not_implemented("MySQLConnection::exec")); }
    Result<ExecResult> MySQLConnection::exec(const std::string &, const std::vector<Value> &) { return Result<ExecResult>::error(Status::not_implemented("MySQLConnection::exec(params)")); }
    Result<std::vector<Row>> MySQLConnection::query(const std::string &) { return Result<std::vector<Row>>::error(Status::not_implemented("MySQLConnection::query")); }
    Result<std::vector<Row>> MySQLConnection::query(const std::string &, const std::vector<Value> &) { return Result<std::vector<Row>>::error(Status::not_implemented("MySQLConnection::query(params)")); }

} // namespace db

#endif
