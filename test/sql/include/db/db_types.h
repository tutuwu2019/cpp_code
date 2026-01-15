#pragma once

#include <cstdint>
#include <string>

namespace db
{

    enum class DBType : uint8_t
    {
        SQLite = 0,
        MySQL = 1,
    };

    inline std::string to_string(DBType t)
    {
        switch (t)
        {
        case DBType::SQLite:
            return "SQLite";
        case DBType::MySQL:
            return "MySQL";
        default:
            return "Unknown";
        }
    }

} // namespace db
