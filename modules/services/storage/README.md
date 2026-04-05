# Storage 服务模块

## 功能说明

数据持久化服务模块，提供通用的数据存储和查询功能。

## 依赖关系

- 基础公共库
- 工具库

## 主要功能

- 数据库连接管理
- SQL 执行
- 预处理语句支持
- 事务支持
- 多种数据类型支持（整数、浮点数、文本、二进制）
- 结果集处理

## 使用示例

```c
// 打开数据库
idcu_StorageDatabase* db;
idcu_storage_open("data.db", &db);

// 执行 SQL
idcu_storage_execute(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)");

// 事务处理
idcu_storage_begin_transaction(db);
idcu_storage_execute(db, "INSERT INTO users (name) VALUES ('Test')");
idcu_storage_commit_transaction(db);

// 关闭数据库
idcu_storage_close(db);
```
