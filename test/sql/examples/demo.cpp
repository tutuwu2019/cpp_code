#include "db/db_helper.h"
#include "db/sqlite_connection.h"
#include "db/user_repository.h"
#include "db/user.h"

#include <iostream>
#include <sstream>

static void applyUserSchema(db::SQLiteConnection &c)
{
    const char *ddl = R"SQL(
CREATE TABLE IF NOT EXISTS user (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL,
    password_hash TEXT NOT NULL,
    email TEXT,
    phone TEXT,
    nickname TEXT,
    avatar_url TEXT,
    status INTEGER NOT NULL DEFAULT 1,
    is_deleted INTEGER NOT NULL DEFAULT 0,
    last_login_at TEXT,
    last_login_ip TEXT,
    created_at TEXT NOT NULL DEFAULT(datetime('now')),
    updated_at TEXT NOT NULL DEFAULT(datetime('now')),
    deleted_at TEXT
);
CREATE INDEX IF NOT EXISTS idx_user_created_at ON user (created_at);
)SQL";

    auto r = c.exec(ddl);
    if (!r.ok())
    {
        std::cerr << "apply schema failed: " << r.status.message << "\n";
    }
}

int main()
{
    db::DBConfig cfg;
    cfg.type = db::DBType::SQLite;
    // 用文件库避免":memory:"多连接不共享数据的问题
    cfg.sqlite.file = "./demo.db";

    // Enable WAL mode for better concurrent read/write (when supported)
    cfg.sqlite.walEnabled = true;
    cfg.sqlite.synchronousNormal = true;

    db::SQLiteConnection sqlite(cfg.sqlite);
    auto s = sqlite.connect();
    if (!s.ok())
    {
        std::cerr << "sqlite connect failed: " << s.message << "\n";
        return 1;
    }

    applyUserSchema(sqlite);

    db::DBHelper helper(cfg);

    // --- 批量插入 ---
    db::UserRepository repo(helper);
    for (int i = 0; i < 20; ++i)
    {
        db::User u;
        u.username = (i % 2 == 0) ? "alice" : "bob";
        u.password_hash = "hash_" + std::to_string(i);
        u.email = u.username + std::to_string(i) + "@example.com";

        auto r = repo.create(u);
        if (!r.ok())
        {
            std::cerr << "batch create failed: " << r.status.message << "\n";
            return 1;
        }
    }

    // --- 业务查询：按用户名 + 创建时间过滤 ---
    // 这里 created_at 是 SQLite 默认 datetime('now') 字符串（YYYY-MM-DD HH:MM:SS）
    db::UserQuery q1;
    q1.username = "alice";
    q1.created_from = "1970-01-01 00:00:00";
    q1.created_to = "2999-12-31 23:59:59";
    q1.limit = 100;

    auto qr = repo.query(q1);
    if (!qr.ok())
    {
        std::cerr << "repo.query failed: " << qr.status.message << "\n";
        return 1;
    }
    std::cout << "query(alice) size=" << qr.value.size() << "\n";

    // --- 更新一条 ---
    if (!qr.value.empty())
    {
        auto u = qr.value.front();
        u.password_hash = "updated_hash";
        auto ur = repo.updateById(u);
        if (!ur.ok())
        {
            std::cerr << "updateById failed: " << ur.status.message << "\n";
            return 1;
        }
        std::cout << "updated rows=" << ur.value << "\n";
    }

    // --- 删除两条（软删）---
    if (qr.value.size() >= 2)
    {
        for (int k = 0; k < 2; ++k)
        {
            auto dr = repo.deleteById(qr.value[k].id);
            if (!dr.ok())
            {
                std::cerr << "deleteById failed: " << dr.status.message << "\n";
                return 1;
            }
            std::cout << "deleted rows=" << dr.value << " (id=" << qr.value[k].id << ")\n";
        }
    }

    // --- 再查一次确认删除生效 ---
    auto qr2 = repo.query(q1);
    if (!qr2.ok())
    {
        std::cerr << "repo.query after delete failed: " << qr2.status.message << "\n";
        return 1;
    }
    std::cout << "query(alice) after delete size=" << qr2.value.size() << "\n";

    // --- 直接用底层查询输出部分行 ---
    auto q = sqlite.query("SELECT id, username, email, created_at, is_deleted FROM user ORDER BY id LIMIT 5;");
    if (!q.ok())
    {
        std::cerr << "query failed: " << q.status.message << "\n";
        return 1;
    }

    for (const auto &row : q.value)
    {
        auto id = std::get<int64_t>(row.at("id"));
        auto username = std::get<std::string>(row.at("username"));
        std::string email;
        if (auto it = row.find("email"); it != row.end() && std::holds_alternative<std::string>(it->second))
        {
            email = std::get<std::string>(it->second);
        }
        std::cout << "row: id=" << id << ", username=" << username << ", email=" << email << "\n";
    }

    return 0;
}
