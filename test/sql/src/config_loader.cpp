#include "db/config_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace db
{
    namespace
    {
        static inline void ltrim(std::string &s)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                            { return !std::isspace(ch); }));
        }

        static inline void rtrim(std::string &s)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                                 { return !std::isspace(ch); })
                        .base(),
                    s.end());
        }

        static inline void trim(std::string &s)
        {
            ltrim(s);
            rtrim(s);
        }

        static inline bool parse_bool(const std::string &v, bool &out)
        {
            std::string s = v;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            if (s == "1" || s == "true" || s == "yes" || s == "on")
            {
                out = true;
                return true;
            }
            if (s == "0" || s == "false" || s == "no" || s == "off")
            {
                out = false;
                return true;
            }
            return false;
        }

        static inline bool starts_with(const std::string &s, const char *prefix)
        {
            const std::string p(prefix);
            return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
        }
    } // namespace

    Status load_db_config_from_file(const std::string &path, DBConfig &out)
    {
        std::ifstream in(path);
        if (!in.is_open())
        {
            return Status::driver_error("failed to open config file: " + path);
        }

        std::string line;
        std::size_t lineNo = 0;
        while (std::getline(in, line))
        {
            ++lineNo;
            // strip comments
            auto hashPos = line.find('#');
            if (hashPos != std::string::npos)
                line.resize(hashPos);
            trim(line);
            if (line.empty())
                continue;

            auto eqPos = line.find('=');
            if (eqPos == std::string::npos)
            {
                return Status::invalid_argument("config parse error at line " + std::to_string(lineNo) + ": missing '='");
            }

            std::string key = line.substr(0, eqPos);
            std::string val = line.substr(eqPos + 1);
            trim(key);
            trim(val);

            if (key == "type")
            {
                std::string v = val;
                std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c)
                               { return static_cast<char>(std::tolower(c)); });
                if (v == "sqlite")
                    out.type = DBType::SQLite;
                else if (v == "mysql")
                    out.type = DBType::MySQL;
                else
                    return Status::invalid_argument("unknown type: " + val);
                continue;
            }

            if (starts_with(key, "mysql."))
            {
                const std::string sub = key.substr(6);
                if (sub == "host")
                    out.mysql.host = val;
                else if (sub == "port")
                    out.mysql.port = static_cast<uint16_t>(std::stoul(val));
                else if (sub == "user")
                    out.mysql.user = val;
                else if (sub == "password")
                    out.mysql.password = val;
                else if (sub == "database")
                    out.mysql.database = val;
                else if (sub == "maxSize")
                    out.mysql.maxSize = static_cast<std::size_t>(std::stoul(val));
                else
                    return Status::invalid_argument("unknown mysql key: " + key);
                continue;
            }

            if (starts_with(key, "sqlite."))
            {
                const std::string sub = key.substr(7);
                if (sub == "file")
                    out.sqlite.file = val;
                else if (sub == "walEnabled")
                {
                    bool b;
                    if (!parse_bool(val, b))
                        return Status::invalid_argument("invalid bool for " + key + ": " + val);
                    out.sqlite.walEnabled = b;
                }
                else if (sub == "synchronousNormal")
                {
                    bool b;
                    if (!parse_bool(val, b))
                        return Status::invalid_argument("invalid bool for " + key + ": " + val);
                    out.sqlite.synchronousNormal = b;
                }
                else
                    return Status::invalid_argument("unknown sqlite key: " + key);
                continue;
            }

            return Status::invalid_argument("unknown config key: " + key);
        }

        return Status::success();
    }

} // namespace db
