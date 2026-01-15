cmake_minimum_required(VERSION 3.15)
project(test_260115_db_components CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DB_SQLITE_WAL "Enable SQLite WAL mode by default" ON)

add_library(db_components
    src/sqlite_connection.cpp
    src/mysql_connection.cpp
    src/mysql_connection_pool.cpp
    src/db_helper.cpp
    src/user_repository.cpp
    src/config_loader.cpp
)

target_include_directories(db_components PUBLIC include)

if (DB_SQLITE_WAL)
    target_compile_definitions(db_components PUBLIC DB_SQLITE_WAL=1)
endif()

find_package(SQLite3)
if (SQLite3_FOUND)
    target_compile_definitions(db_components PUBLIC DB_HAS_SQLITE3=1)
    target_link_libraries(db_components PUBLIC SQLite::SQLite3)
else()
    message(WARNING "SQLite3 not found. SQLite-based implementation will be stubbed; demo_db will be disabled.")
endif()

# --- mysqlclient (C API) ---
find_path(MYSQL_INCLUDE_DIR mysql.h PATH_SUFFIXES mysql PATHS /usr/include /usr/local/include)
find_library(MYSQL_CLIENT_LIB mysqlclient
    PATHS /usr/lib64 /usr/lib64/mysql /usr/local/lib64 /usr/local/lib
)
if (MYSQL_INCLUDE_DIR AND MYSQL_CLIENT_LIB)
    target_compile_definitions(db_components PUBLIC DB_HAS_MYSQLCLIENT=1)
    # mysql.h lives under /usr/include/mysql/mysql.h, so we need include root (/usr/include/mysql)
    target_include_directories(db_components PUBLIC ${MYSQL_INCLUDE_DIR})
    target_link_libraries(db_components PUBLIC ${MYSQL_CLIENT_LIB})
else()
    message(STATUS "mysqlclient not found; MySQL backend stays as stub.")
endif()

if (SQLite3_FOUND)
    add_executable(demo_db examples/demo.cpp)
    target_link_libraries(demo_db PRIVATE db_components)
endif()

if (MYSQL_INCLUDE_DIR AND MYSQL_CLIENT_LIB)
    add_executable(demo_mysql examples/demo_mysql.cpp)
    target_link_libraries(demo_mysql PRIVATE db_components)
    target_link_libraries(demo_mysql PRIVATE Threads::Threads)
    target_compile_definitions(demo_mysql PRIVATE DB_MYSQL_POOL_DEBUG=1)
endif()
