#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace db
{

    struct User
    {
        int64_t id{0};
        std::string username;
        std::string password_hash;
        std::optional<std::string> email;
        std::optional<std::string> phone;
        std::optional<std::string> nickname;
        std::optional<std::string> avatar_url;

        int status{1};
        int is_deleted{0};

        // store as ISO8601 string or SQLite datetime('now') string
        std::string created_at;
        std::string updated_at;
    };

    struct UserQuery
    {
        // filter by username exact match when set
        std::optional<std::string> username;

        // created_at range filter (inclusive). When unset, ignored.
        std::optional<std::string> created_from;
        std::optional<std::string> created_to;

        // pagination
        int limit{100};
        int offset{0};
    };

} // namespace db
