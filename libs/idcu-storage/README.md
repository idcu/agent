# idcu-storage

IDCU 项目的存储服务库，提供数据持久化和 SQL 数据库访问功能。

## 特性

- SQL 数据库操作
- 预编译语句支持
- 参数绑定（整数、浮点数、文本、二进制）
- 事务支持
- 线程安全
- 跨平台支持（Windows、Linux、macOS）

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-storage REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::storage)
```

## API 文档

### 初始化和清理

```c
#include <idcu/storage/storage.h>

int idcu_storage_init(void);
void idcu_storage_cleanup(void);
```

### 数据库管理

```c
int idcu_storage_open(const char* db_path, idcu_StorageDatabase** db);
int idcu_storage_close(idcu_StorageDatabase* db);
```

### SQL 执行

```c
int idcu_storage_execute(idcu_StorageDatabase* db, const char* sql);
int idcu_storage_execute_format(idcu_StorageDatabase* db, const char* format, ...);
```

### 预编译语句

```c
int idcu_storage_prepare(idcu_StorageDatabase* db, const char* sql, idcu_StorageStatement** stmt);
int idcu_storage_bind_int(idcu_StorageStatement* stmt, int index, int64_t value);
int idcu_storage_bind_float(idcu_StorageStatement* stmt, int index, double value);
int idcu_storage_bind_text(idcu_StorageStatement* stmt, int index, const char* value);
int idcu_storage_bind_blob(idcu_StorageStatement* stmt, int index, const void* value, size_t size);
int idcu_storage_bind_null(idcu_StorageStatement* stmt, int index);
```

### 结果处理

```c
int idcu_storage_step(idcu_StorageStatement* stmt, idcu_StorageResult** result);
int idcu_storage_column_count(idcu_StorageResult* result);
const char* idcu_storage_column_name(idcu_StorageResult* result, int index);
idcu_StorageType idcu_storage_column_type(idcu_StorageResult* result, int index);
int64_t idcu_storage_column_int(idcu_StorageResult* result, int index);
double idcu_storage_column_float(idcu_StorageResult* result, int index);
const char* idcu_storage_column_text(idcu_StorageResult* result, int index);
const void* idcu_storage_column_blob(idcu_StorageResult* result, int index, size_t* size);
```

### 资源清理

```c
int idcu_storage_result_destroy(idcu_StorageResult* result);
int idcu_storage_finalize(idcu_StorageStatement* stmt);
```

### 事务和工具

```c
int64_t idcu_storage_last_insert_rowid(idcu_StorageDatabase* db);
int idcu_storage_changes(idcu_StorageDatabase* db);
int idcu_storage_begin_transaction(idcu_StorageDatabase* db);
int idcu_storage_commit_transaction(idcu_StorageDatabase* db);
int idcu_storage_rollback_transaction(idcu_StorageDatabase* db);
```

## 测试

```bash
cd build
ctest
```

## 示例

库提供了以下示例代码：

- [基础存储示例](examples/example_storage_basic.c) - 演示基本的数据库操作
- [事务示例](examples/example_storage_transaction.c) - 演示事务的使用

### 编译和运行示例

```bash
cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .

# 运行基础存储示例
./example_storage_basic

# 运行事务示例
./example_storage_transaction
```

## 许可证

详见项目根目录的 LICENSE 文件。
