#include "db/mysql_connection_pool.h"

#include "db/db_result.h"
#include "db/mysql_connection.h"

#include <iostream>

namespace db
{

    MySQLConnectionPool::MySQLConnectionPool(MySQLConfig cfg) : cfg_(std::move(cfg)) {}

    Result<std::shared_ptr<MySQLConnection>> MySQLConnectionPool::acquire(Duration timeout)
    {
        std::unique_lock<std::mutex> lk(mu_);

        // 1) idle 非空
        if (!idle_.empty())
        {
            auto conn = idle_.back();
            idle_.pop_back();
#if DB_MYSQL_POOL_DEBUG
            std::cerr << "[mysql-pool] acquire: reuse idle (idle=" << idle_.size() << ", total=" << total_ << ")\n";
#endif
            return Result<std::shared_ptr<MySQLConnection>>::ok(std::move(conn));
        }

        // 2) idle 为空但未达 maxSize：创建新连接
        if (total_ < cfg_.maxSize)
        {
            auto conn = std::make_shared<MySQLConnection>(cfg_);
            ++total_;
#if DB_MYSQL_POOL_DEBUG
            std::cerr << "[mysql-pool] acquire: create new (idle=" << idle_.size() << ", total=" << total_ << ")\n";
#endif
            return Result<std::shared_ptr<MySQLConnection>>::ok(std::move(conn));
        }

        // 3) idle 为空且已达 maxSize：等待
        bool notified = cv_.wait_for(lk, timeout, [&]
                                     { return !idle_.empty(); });
        if (!notified)
        {
#if DB_MYSQL_POOL_DEBUG
            std::cerr << "[mysql-pool] acquire: timeout (idle=" << idle_.size() << ", total=" << total_ << ")\n";
#endif
            return Result<std::shared_ptr<MySQLConnection>>::error(Status::timeout("MySQLConnectionPool acquire timeout"));
        }

        auto conn = idle_.back();
        idle_.pop_back();
#if DB_MYSQL_POOL_DEBUG
        std::cerr << "[mysql-pool] acquire: woke and got idle (idle=" << idle_.size() << ", total=" << total_ << ")\n";
#endif
        return Result<std::shared_ptr<MySQLConnection>>::ok(std::move(conn));
    }

    void MySQLConnectionPool::release(std::shared_ptr<MySQLConnection> conn)
    {
        if (!conn)
            return;

        {
            std::lock_guard<std::mutex> lk(mu_);
            idle_.push_back(std::move(conn));
#if DB_MYSQL_POOL_DEBUG
            std::cerr << "[mysql-pool] release: return to idle (idle=" << idle_.size() << ", total=" << total_ << ")\n";
#endif
        }
        cv_.notify_one();
    }

} // namespace db
