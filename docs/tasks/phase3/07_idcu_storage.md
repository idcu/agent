# 任务 3.7: idcu-storage - 持久化存储库

## 目标

创建持久化存储库，支持：
- 键值存储
- SQLite 数据库封装
- 事务支持
- 数据序列化
- 查询接口
- 线程安全

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-storage/include/idcu/storage
mkdir -p libs/idcu-storage/src/idcu/storage
mkdir -p libs/idcu-storage/tests
mkdir -p libs/idcu-storage/examples
```

### 2. 创建存储头文件 (storage.h)

创建 `libs/idcu-storage/include/idcu/storage/storage.h`：

```c
#ifndef IDCU_STORAGE_STORAGE_H
#define IDCU_STORAGE_STORAGE_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_STORAGE_TYPE_KV = 0,
    IDCU_STORAGE_TYPE_SQLITE
} idcu_StorageType;

typedef struct idcu_KVStore idcu_KVStore;
typedef struct idcu_SQLiteDB idcu_SQLiteDB;
typedef struct idcu_Transaction idcu_Transaction;

struct idcu_KVStore
{
    char path[1024];
    idcu_HashMap data;
    idcu_Mutex lock;
    int dirty;
    int initialized;
};

struct idcu_SQLiteDB
{
    void* db_handle;
    char path[1024];
    idcu_Mutex lock;
    int in_transaction;
    int initialized;
};

struct idcu_Transaction
{
    idcu_SQLiteDB* db;
    int active;
};

int  idcu_kvstore_init(idcu_KVStore* store, const char* path);
void idcu_kvstore_destroy(idcu_KVStore* store);
int  idcu_kvstore_put(idcu_KVStore* store, const char* key, const void* value, size_t value_size);
int  idcu_kvstore_put_string(idcu_KVStore* store, const char* key, const char* value);
int  idcu_kvstore_put_int(idcu_KVStore* store, const char* key, int64_t value);
int  idcu_kvstore_put_double(idcu_KVStore* store, const char* key, double value);
int  idcu_kvstore_get(idcu_KVStore* store, const char* key, void* buffer, size_t buffer_size, size_t* value_size);
int  idcu_kvstore_get_string(idcu_KVStore* store, const char* key, char* buffer, size_t buffer_size);
int  idcu_kvstore_get_int(idcu_KVStore* store, const char* key, int64_t* value);
int  idcu_kvstore_get_double(idcu_KVStore* store, const char* key, double* value);
int  idcu_kvstore_remove(idcu_KVStore* store, const char* key);
int  idcu_kvstore_exists(idcu_KVStore* store, const char* key);
int  idcu_kvstore_clear(idcu_KVStore* store);
size_t idcu_kvstore_count(idcu_KVStore* store);
int  idcu_kvstore_sync(idcu_KVStore* store);
int  idcu_kvstore_load(idcu_KVStore* store);

int  idcu_sqlite_init(idcu_SQLiteDB* db, const char* path);
void idcu_sqlite_destroy(idcu_SQLiteDB* db);
int  idcu_sqlite_execute(idcu_SQLiteDB* db, const char* sql);
int  idcu_sqlite_query(idcu_SQLiteDB* db, const char* sql, idcu_Vector* results);
int  idcu_sqlite_prepare(idcu_SQLiteDB* db, const char* sql, void** stmt);
int  idcu_sqlite_bind_text(void* stmt, int index, const char* value);
int  idcu_sqlite_bind_int(void* stmt, int index, int64_t value);
int  idcu_sqlite_bind_double(void* stmt, int index, double value);
int  idcu_sqlite_bind_blob(void* stmt, int index, const void* value, size_t size);
int  idcu_sqlite_bind_null(void* stmt, int index);
int  idcu_sqlite_step(void* stmt);
int  idcu_sqlite_column_count(void* stmt);
const char* idcu_sqlite_column_name(void* stmt, int index);
int  idcu_sqlite_column_type(void* stmt, int index);
const char* idcu_sqlite_column_text(void* stmt, int index);
int64_t idcu_sqlite_column_int(void* stmt, int index);
double idcu_sqlite_column_double(void* stmt, int index);
const void* idcu_sqlite_column_blob(void* stmt, int index, size_t* size);
int  idcu_sqlite_finalize(void* stmt);
int64_t idcu_sqlite_last_insert_rowid(idcu_SQLiteDB* db);
int64_t idcu_sqlite_changes(idcu_SQLiteDB* db);

int  idcu_sqlite_begin_transaction(idcu_SQLiteDB* db);
int  idcu_sqlite_commit(idcu_SQLiteDB* db);
int  idcu_sqlite_rollback(idcu_SQLiteDB* db);

int  idcu_transaction_init(idcu_Transaction* tx, idcu_SQLiteDB* db);
void idcu_transaction_destroy(idcu_Transaction* tx);
int  idcu_transaction_begin(idcu_Transaction* tx);
int  idcu_transaction_commit(idcu_Transaction* tx);
int  idcu_transaction_rollback(idcu_Transaction* tx);

typedef struct
{
    idcu_Vector columns;
    idcu_Vector values;
} idcu_SQLiteRow;

int  idcu_sqlite_row_init(idcu_SQLiteRow* row);
void idcu_sqlite_row_destroy(idcu_SQLiteRow* row);
int  idcu_sqlite_row_add_column(idcu_SQLiteRow* row, const char* name);
int  idcu_sqlite_row_add_value(idcu_SQLiteRow* row, const char* value);
const char* idcu_sqlite_row_get_value(idcu_SQLiteRow* row, const char* column);
const char* idcu_sqlite_row_get_value_by_index(idcu_SQLiteRow* row, size_t index);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-storage/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-storage VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-storage STATIC
    src/idcu/storage/storage.c
)

target_include_directories(idcu-storage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-storage PRIVATE
    idcu::common
    idcu::json
)

find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    target_link_libraries(idcu-storage PRIVATE SQLite::SQLite3)
else()
    message(STATUS "SQLite3 not found, using embedded version or disabling SQLite features")
endif()

add_library(idcu::storage ALIAS idcu-storage)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-storage/module.yaml`：

```yaml
name: idcu-storage
version: 1.0.0
description: Persistent storage library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-json

build:
  type: cmake
  targets:
    - idcu-storage

headers:
  - idcu/storage/storage.h

features:
  - kvstore: Key-value store
  - sqlite: SQLite database wrapper
  - transaction: Transaction support
  - serialization: Data serialization
  - query: Query interface
  - thread_safe: Thread-safe operations

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-storage/README.md`：

```markdown
# idcu-storage

IDCU Agent 的持久化存储库。

## 功能特性

- **键值存储**: 简单的键值存储
- **SQLite 封装**: SQLite 数据库封装
- **事务支持**: 数据库事务支持
- **数据序列化**: 数据序列化
- **查询接口**: 查询接口
- **线程安全**: 线程安全操作

## 快速开始

### 键值存储

```c
#include "idcu/storage/storage.h"

idcu_KVStore store;
idcu_kvstore_init(&store, "./data.kv");

idcu_kvstore_put_string(&store, "name", "John");
idcu_kvstore_put_int(&store, "age", 30);
idcu_kvstore_put_double(&store, "score", 95.5);

char name[256];
idcu_kvstore_get_string(&store, "name", name, sizeof(name));

int64_t age;
idcu_kvstore_get_int(&store, "age", &age);

idcu_kvstore_sync(&store);

idcu_kvstore_destroy(&store);
```

### SQLite 数据库

```c
idcu_SQLiteDB db;
idcu_sqlite_init(&db, "./data.db");

idcu_sqlite_execute(&db, 
    "CREATE TABLE IF NOT EXISTS users ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "name TEXT NOT NULL,"
    "age INTEGER"
    ")");

idcu_Transaction tx;
idcu_transaction_init(&tx, &db);
idcu_transaction_begin(&tx);

idcu_sqlite_execute(&db, "INSERT INTO users (name, age) VALUES ('John', 30)");

idcu_transaction_commit(&tx);
idcu_transaction_destroy(&tx);

idcu_Vector results;
idcu_vector_init(&results, sizeof(idcu_SQLiteRow));
idcu_sqlite_query(&db, "SELECT * FROM users", &results);

idcu_sqlite_destroy(&db);
```

### SQLite 预处理语句

```c
void* stmt;
idcu_sqlite_prepare(&db, "INSERT INTO users (name, age) VALUES (?, ?)", &stmt);

idcu_sqlite_bind_text(stmt, 1, "Jane");
idcu_sqlite_bind_int(stmt, 2, 25);

idcu_sqlite_step(stmt);
idcu_sqlite_finalize(stmt);

int64_t last_id = idcu_sqlite_last_insert_rowid(&db);
```

## API 文档

详见 [include/idcu/storage/storage.h](include/idcu/storage/storage.h)
```

## 验证检查清单

- [ ] 存储头文件已创建
- [ ] 存储实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 键值存储可以正常读写
- [ ] SQLite 数据库可以正常操作
- [ ] 事务可以正常提交和回滚

## Git 提交

```bash
git add libs/idcu-storage/
git commit -m "feat: add idcu-storage library

- Add key-value store
- Add SQLite database wrapper
- Add transaction support
- Add data serialization
- Add query interface
- Add thread-safe operations
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 数据库锁定 | 多线程未正确使用锁 | 确保在多线程环境中正确使用 |
| 数据丢失 | 未调用 sync | 确保修改后调用 sync |
| SQLite 错误 | SQL 语法错误 | 检查 SQL 语句语法 |
