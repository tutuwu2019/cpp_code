# test_260115 - 数据库基础组件（C++17）

本目录根据 `doc/Diagram.md` 的时序图实现一套“数据库基础组件”的骨架：

- `DBHelper`：对外提供业务侧入口（示例：`createUser`）
- `IDatabase`：抽象数据库接口（`exec/query`）
- `SQLiteConnection`：SQLite 具体实现（可运行）
- `MySQLConnectionPool`：连接池（按 `doc/connectionPool.md` 的 acquire/release 逻辑实现）
- `MySQLConnection`：MySQL连接对象（当前仅提供接口骨架，未集成真实驱动）

## 目录结构

- `include/db/`：头文件
- `src/`：实现
- `examples/demo.cpp`：最小演示（SQLite）

## 依赖

- CMake >= 3.15
- SQLite3 开发库

## 构建与运行

在本目录下：

```bash
cmake -S . -B build
cmake --build build -j
./build/demo_db
```

## 说明

- 按时序图，MySQL路径需要：`MySQLAdapter -> Pool.acquire() -> Conn.exec() -> Pool.release()`。
  目前为了做到“组件可编译可演示”，MySQL驱动未接入，调用会返回 `NotImplemented`。
- 若你希望接入 `mysqlclient` 或 `mysql-connector-c++`，我可以在现有接口不变的前提下补齐实现与CMake查找/链接。
