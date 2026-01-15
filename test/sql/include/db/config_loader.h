#pragma once

#include "db/db_config.h"
#include "db/db_error.h"

#include <string>

namespace db
{

    // A tiny, dependency-free config loader for demos.
    // File format: one "key=value" per line, '#' starts a comment.
    // Example:
    //   type=mysql
    //   mysql.host=127.0.0.1
    //   mysql.port=3306
    //   mysql.user=root
    //   mysql.password=123456
    //   mysql.database=test
    //   mysql.maxSize=5
    //   sqlite.file=./demo.db
    //   sqlite.walEnabled=true
    //   sqlite.synchronousNormal=true
    Status load_db_config_from_file(const std::string &path, DBConfig &out);

} // namespace db
