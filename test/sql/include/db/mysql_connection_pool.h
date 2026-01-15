#pragma once

#include "db/db_config.h"
#include "db/db_error.h"
#include "db/db_result.h"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

// Define DB_MYSQL_POOL_DEBUG=1 at compile time to enable pool logs.
#ifndef DB_MYSQL_POOL_DEBUG
#define DB_MYSQL_POOL_DEBUG 0
#endif

namespace db
{

    class MySQLConnection;

    class MySQLConnectionPool
    {
    public:
        using Duration = std::chrono::milliseconds;

        explicit MySQLConnectionPool(MySQLConfig cfg);

        // acquire according to Diagram: idle->pop, else create until maxSize, else wait with timeout
        Result<std::shared_ptr<MySQLConnection>> acquire(Duration timeout);
        void release(std::shared_ptr<MySQLConnection> conn);

    private:
        MySQLConfig cfg_;

        std::mutex mu_;
        std::condition_variable cv_;

        std::vector<std::shared_ptr<MySQLConnection>> idle_;
        std::size_t total_{0};
    };

} // namespace db
