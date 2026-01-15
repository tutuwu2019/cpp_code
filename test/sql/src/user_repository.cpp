#include "db/user_repository.h"

#include <optional>
#include <sstream>

namespace db
{

    static std::optional<std::string> getOptString(const Row &row, const std::string &key)
    {
        auto it = row.find(key);
        if (it == row.end())
            return std::nullopt;
        if (std::holds_alternative<std::string>(it->second))
            return std::get<std::string>(it->second);
        return std::nullopt;
    }

    static int64_t getInt64(const Row &row, const std::string &key)
    {
        auto it = row.find(key);
        if (it == row.end())
            return 0;
        if (std::holds_alternative<int64_t>(it->second))
            return std::get<int64_t>(it->second);
        return 0;
    }

    UserRepository::UserRepository(DBHelper &helper) : helper_(helper) {}

    Result<int64_t> UserRepository::create(const User &u)
    {
        if (u.username.empty() || u.password_hash.empty())
        {
            return Result<int64_t>::error(Status::invalid_argument("username/password_hash empty"));
        }

        std::string sql;
        std::vector<Value> params;

        if (u.email.has_value())
        {
            sql = "INSERT INTO user(username, password_hash, email) VALUES(?, ?, ?);";
            params = {u.username, u.password_hash, *u.email};
        }
        else
        {
            sql = "INSERT INTO user(username, password_hash) VALUES(?, ?);";
            params = {u.username, u.password_hash};
        }

        auto r = helper_.exec(sql, params);
        if (!r.ok())
            return Result<int64_t>::error(std::move(r.status));
        return Result<int64_t>::ok(r.value.lastInsertId);
    }

    Result<User> UserRepository::getById(int64_t id)
    {
        if (id <= 0)
            return Result<User>::error(Status::invalid_argument("id<=0"));

        const std::string sql =
            "SELECT id, username, password_hash, email, phone, nickname, avatar_url, status, is_deleted, created_at, updated_at "
            "FROM user WHERE id=? AND is_deleted=0 LIMIT 1;";

        auto r = helper_.query(sql, {id});
        if (!r.ok())
            return Result<User>::error(std::move(r.status));
        if (r.value.empty())
            return Result<User>::error(Status::driver_error("not found"));

        const auto &row = r.value.front();
        User u;
        u.id = getInt64(row, "id");
        u.username = getOptString(row, "username").value_or("");
        u.password_hash = getOptString(row, "password_hash").value_or("");
        u.email = getOptString(row, "email");
        u.phone = getOptString(row, "phone");
        u.nickname = getOptString(row, "nickname");
        u.avatar_url = getOptString(row, "avatar_url");
        u.status = static_cast<int>(getInt64(row, "status"));
        u.is_deleted = static_cast<int>(getInt64(row, "is_deleted"));
        u.created_at = getOptString(row, "created_at").value_or("");
        u.updated_at = getOptString(row, "updated_at").value_or("");

        return Result<User>::ok(std::move(u));
    }

    Result<std::vector<User>> UserRepository::query(const UserQuery &q)
    {
        std::ostringstream oss;
        oss << "SELECT id, username, password_hash, email, phone, nickname, avatar_url, status, is_deleted, created_at, updated_at "
               "FROM user WHERE is_deleted=0";

        std::vector<Value> params;
        if (q.username && !q.username->empty())
        {
            oss << " AND username=?";
            params.emplace_back(*q.username);
        }
        if (q.created_from && !q.created_from->empty())
        {
            oss << " AND created_at >= ?";
            params.emplace_back(*q.created_from);
        }
        if (q.created_to && !q.created_to->empty())
        {
            oss << " AND created_at <= ?";
            params.emplace_back(*q.created_to);
        }

        oss << " ORDER BY created_at DESC";
        oss << " LIMIT " << q.limit << " OFFSET " << q.offset << ";";

        auto r = helper_.query(oss.str(), params);
        if (!r.ok())
            return Result<std::vector<User>>::error(std::move(r.status));

        std::vector<User> out;
        out.reserve(r.value.size());
        for (const auto &row : r.value)
        {
            User u;
            u.id = getInt64(row, "id");
            u.username = getOptString(row, "username").value_or("");
            u.password_hash = getOptString(row, "password_hash").value_or("");
            u.email = getOptString(row, "email");
            u.phone = getOptString(row, "phone");
            u.nickname = getOptString(row, "nickname");
            u.avatar_url = getOptString(row, "avatar_url");
            u.status = static_cast<int>(getInt64(row, "status"));
            u.is_deleted = static_cast<int>(getInt64(row, "is_deleted"));
            u.created_at = getOptString(row, "created_at").value_or("");
            u.updated_at = getOptString(row, "updated_at").value_or("");
            out.emplace_back(std::move(u));
        }

        return Result<std::vector<User>>::ok(std::move(out));
    }

    Result<int64_t> UserRepository::updateById(const User &u)
    {
        if (u.id <= 0)
            return Result<int64_t>::error(Status::invalid_argument("id<=0"));

        std::ostringstream oss;
        std::vector<Value> params;

        oss << "UPDATE user SET username=?, password_hash=?, updated_at=datetime('now')";
        params.emplace_back(u.username);
        params.emplace_back(u.password_hash);

        if (u.email.has_value())
        {
            oss << ", email=?";
            params.emplace_back(*u.email);
        }

        oss << " WHERE id=? AND is_deleted=0;";
        params.emplace_back(u.id);

        auto r = helper_.exec(oss.str(), params);
        if (!r.ok())
            return Result<int64_t>::error(std::move(r.status));
        return Result<int64_t>::ok(r.value.affectedRows);
    }

    Result<int64_t> UserRepository::deleteById(int64_t id)
    {
        if (id <= 0)
            return Result<int64_t>::error(Status::invalid_argument("id<=0"));

        const std::string sql =
            "UPDATE user SET is_deleted=1, deleted_at=datetime('now'), updated_at=datetime('now') "
            "WHERE id=? AND is_deleted=0;";

        auto r = helper_.exec(sql, {id});
        if (!r.ok())
            return Result<int64_t>::error(std::move(r.status));
        return Result<int64_t>::ok(r.value.affectedRows);
    }

} // namespace db
