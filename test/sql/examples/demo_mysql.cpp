#include "db/db_helper.h"
#include "db/config_loader.h"
#include "db/user_repository.h"

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

static void usage(const char *argv0)
{
    std::cerr << "usage: " << argv0 << " [config_path]\n";
    std::cerr << "config format: key=value per line, '#' comment\n";
}

// 说明：该 demo 仅在系统装了 mysql-devel (mysqlclient) 时会被构建。
// 运行前需要准备 MySQL 表结构（可用 doc/user.sql 改写为 MySQL 版），并正确设置环境变量。
//
// 环境变量：
//   DB_MYSQL_HOST / DB_MYSQL_PORT / DB_MYSQL_USER / DB_MYSQL_PASSWORD / DB_MYSQL_DATABASE
//
// 注意：MySQL 版的 table DDL 与时间函数（NOW()）与 SQLite 不同，这里 demo 不自动建表。

int main(int argc, char **argv)
{
    const std::string cfgPath = (argc >= 2) ? std::string(argv[1]) : std::string("./db_config.demo.conf");
    if (argc > 2)
    {
        usage(argv[0]);
        return 2;
    }
    {
        db::DBConfig cfg;
        auto stCfg = db::load_db_config_from_file(cfgPath, cfg);
        if (!stCfg.ok())
        {
            std::cerr << "load config failed: " << stCfg.message << "\n";
            std::cerr << "config path: " << cfgPath << "\n";
            std::cerr << "(tip) copy ./db_config.demo.conf.example -> ./db_config.demo.conf and edit it\n";
            return 2;
        }
        if (cfg.type != db::DBType::MySQL)
        {
            std::cerr << "config type must be mysql for this demo\n";
            return 2;
        }

        db::DBHelper helper(cfg);

        // Concurrent workload to visibly exercise the connection pool.
        const int threads = 8;
        const int itersPerThread = 20;

        std::atomic<int> okCreates{0};
        std::atomic<int> okQueries{0};
        std::atomic<int> failures{0};

        auto start = std::chrono::steady_clock::now();
        std::vector<std::thread> workers;
        workers.reserve(threads);

        for (int t = 0; t < threads; ++t)
        {
            workers.emplace_back([&, t]
                                 {
                                     db::UserRepository repo(helper);

                                     for (int i = 0; i < itersPerThread; ++i)
                                     {
                                         db::User u;
                                         u.username = "alice"; // fixed username to make query hit
                                         u.password_hash = "hash_mysql";
                                         u.email = "alice_mysql@example.com";
                                         u.nickname = "t" + std::to_string(t) + "_" + std::to_string(i);

                                         auto cr = repo.create(u);
                                         if (cr.ok())
                                         {
                                             ++okCreates;
                                         }
                                         else
                                         {
                                             ++failures;
                                             // Print only first few failures to avoid spam.
                                             if (failures.load() <= 3)
                                                 std::cerr << "create failed: " << cr.status.message << "\n";
                                         }

                                         db::UserQuery q;
                                         q.username = "alice";
                                         q.limit = 5;
                                         auto qr = repo.query(q);
                                         if (qr.ok())
                                         {
                                             ++okQueries;
                                         }
                                         else
                                         {
                                             ++failures;
                                             if (failures.load() <= 3)
                                                 std::cerr << "query failed: " << qr.status.message << "\n";
                                         }
                                     }
                                 });
        }

        for (auto &th : workers)
            th.join();

        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "threads=" << threads << " iters/thread=" << itersPerThread << "\n";
        std::cout << "okCreates=" << okCreates.load() << " okQueries=" << okQueries.load() << " failures=" << failures.load() << "\n";
        std::cout << "elapsedMs=" << ms << "\n";

        return failures.load() == 0 ? 0 : 1;
    }
}
