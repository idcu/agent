# idcu-storage

持久化存储库 for IDCU Agent.

## 功能特性

### KV存储（键值存储）
- **内存存储**: 高效内存键值存储
- **JSON持久化**: 自动将数据保存到JSON格式文件
- **自动扩容**: 支持动态扩容，最多10000个条目
- **类型支持**: 支持string、int、double、二进制数据
- **线程安全**: 每个存储实例独立互斥锁
- **脏标记**: 自动同步，或手动同步

### SQLite数据库（简化版）
- **SQL执行**: 支持执行SQL语句
- **事务支持**: 支持BEGIN/COMMIT/ROLLBACK
- **线程安全**: 每个数据库实例独立互斥锁

## KV存储使用方法

```c
#include <idcu/storage/storage.h>

// 初始化KV存储
idcu_KVStore store;
int ret = idcu_kvstore_init(&store, "data.kv");
if (ret != IDCU_ERR_OK) {
    // 处理错误
}

// 写入字符串
idcu_kvstore_put_string(&store, "username", "admin");

// 写入整数
idcu_kvstore_put_int(&store, "port", 8080);

// 写入浮点数
idcu_kvstore_put_double(&store, "pi", 3.1415926);

// 读取字符串
char buffer[256];
idcu_kvstore_get_string(&store, "username", buffer, sizeof(buffer));
printf("Username: %s\n", buffer);

// 读取整数
int64_t port;
idcu_kvstore_get_int(&store, "port", &port);
printf("Port: %lld\n", (long long)port);

// 同步到磁盘
idcu_kvstore_sync(&store);

// 获取条目数
uint32_t count = idcu_kvstore_count(&store);

// 删除条目
idcu_kvstore_remove(&store, "port");

// 清空存储
idcu_kvstore_clear(&store);

// 销毁存储
idcu_kvstore_destroy(&store);
```

## SQLite使用方法（简化版）

```c
#include <idcu/storage/storage.h>

// 初始化SQLite数据库
idcu_SQLiteDB db;
int ret = idcu_sqlite_init(&db, "data.db");
if (ret != IDCU_ERR_OK) {
    // 处理错误
}

// 开始事务
idcu_sqlite_begin_transaction(&db);

// 执行SQL
idcu_sqlite_execute(&db, "CREATE TABLE users (id INT, name TEXT)");
idcu_sqlite_execute(&db, "INSERT INTO users VALUES (1, 'Alice')");

// 提交事务
idcu_sqlite_commit(&db);

// 销毁数据库
idcu_sqlite_destroy(&db);
```

## KV存储持久化格式

数据以JSON格式保存：

```json
{
  "key1": {"size": 5, "data": "68656c6c6f"},
  "key2": {"size": 8, "data": "0100000000000000"}
}
```

## API参考

### KV存储API
- `idcu_kvstore_init()` - 初始化KV存储
- `idcu_kvstore_destroy()` - 销毁KV存储
- `idcu_kvstore_put()` - 写入二进制数据
- `idcu_kvstore_put_string()` - 写入字符串
- `idcu_kvstore_put_int()` - 写入整数
- `idcu_kvstore_put_double()` - 写入浮点数
- `idcu_kvstore_get()` - 读取二进制数据
- `idcu_kvstore_get_string()` - 读取字符串
- `idcu_kvstore_get_int()` - 读取整数
- `idcu_kvstore_get_double()` - 读取浮点数
- `idcu_kvstore_remove()` - 删除条目
- `idcu_kvstore_sync()` - 同步到磁盘
- `idcu_kvstore_clear()` - 清空存储
- `idcu_kvstore_count()` - 获取条目数

### SQLite API
- `idcu_sqlite_init()` - 初始化数据库
- `idcu_sqlite_destroy()` - 销毁数据库
- `idcu_sqlite_execute()` - 执行SQL
- `idcu_sqlite_begin_transaction()` - 开始事务
- `idcu_sqlite_commit()` - 提交事务
- `idcu_sqlite_rollback()` - 回滚事务

## 性能指标

- KV存储读写延迟：≤ 1ms（平均）
- 支持条目数：≤ 10000
- 单条最大长度：≤ 4096字节

## 依赖

- idcu-common
- idcu-json

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

MIT
