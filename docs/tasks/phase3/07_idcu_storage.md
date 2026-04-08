# 任务 3.7: idcu-storage - 持久化存储库

> **文档版本**: v2.0  
> **最后更新**: 2026-04-08  
> **责任人**: IDCU Team  
> **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建持久化存储库，提供键值存储、SQLite 数据库封装、事务支持、数据序列化、查询接口、线程安全操作，满足键值读写延迟 ≤ 1ms、SQLite 查询延迟 ≤ 10ms、支持 10000+ 键值对的性能要求。

### 1.2 不做什么
- 不实现分布式存储（由独立模块处理）
- 不实现 ORM 映射
- 不实现数据库加密
- 不实现复杂的查询构建器

### 1.3 输入
- 存储文件路径
- 键值数据（key/value）
- SQL 查询语句
- 事务操作指令
- 绑定参数（预处理语句）

### 1.4 输出
- 键值读取结果
- SQLite 查询结果集
- 事务执行状态
- 错误码
- 返回码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-json 库已可用
- SQLite3 库已可用（或使用嵌入式版本）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **键值存储**: 内存 HashMap + 磁盘持久化（JSON 格式）
- **SQLite 封装**: 轻量级 SQLite3 封装，简化常用操作
- **事务支持**: SQLite 原生事务支持
- **数据序列化**: JSON 格式
- **线程安全**: 每个存储实例独立互斥锁

### 2.2 核心逻辑
```
KVStore 初始化：
1. 加载持久化文件（若存在）
2. 构建内存哈希表
3. 初始化互斥锁
4. 标记为初始化完成

KVStore 写入：
1. 获取锁
2. 更新内存哈希表
3. 标记为 dirty
4. 释放锁
5. 可选：异步同步到磁盘

SQLite 查询：
1. 获取锁
2. 准备或执行 SQL 语句
3. 绑定参数（如需要）
4. 执行并获取结果
5. 释放锁
6. 返回结果

事务流程：
1. BEGIN TRANSACTION
2. 执行多个操作
3. COMMIT 或 ROLLBACK
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/storage/storage.h

typedef enum {
    IDCU_STORAGE_TYPE_KV = 0,
    IDCU_STORAGE_TYPE_SQLITE
} idcu_StorageType;

// KV 存储
typedef struct idcu_KVStore {
    char path[1024];
    idcu_HashMap data;
    idcu_Mutex lock;
    int dirty;
    int initialized;
} idcu_KVStore;

int  idcu_kvstore_init(idcu_KVStore* store, const char* path);
void idcu_kvstore_destroy(idcu_KVStore* store);
int  idcu_kvstore_put(idcu_KVStore* store, const char* key, const void* value, size_t value_size);
int  idcu_kvstore_put_string(idcu_KVStore* store, const char* key, const char* value);
int  idcu_kvstore_put_int(idcu_KVStore* store, const char* key, int64_t value);
int  idcu_kvstore_get(idcu_KVStore* store, const char* key, void* buffer, size_t buffer_size, size_t* value_size);
int  idcu_kvstore_get_string(idcu_KVStore* store, const char* key, char* buffer, size_t buffer_size);
int  idcu_kvstore_remove(idcu_KVStore* store, const char* key);
int  idcu_kvstore_sync(idcu_KVStore* store);

// SQLite 数据库
typedef struct idcu_SQLiteDB {
    void* db_handle;
    char path[1024];
    idcu_Mutex lock;
    int in_transaction;
    int initialized;
} idcu_SQLiteDB;

int  idcu_sqlite_init(idcu_SQLiteDB* db, const char* path);
void idcu_sqlite_destroy(idcu_SQLiteDB* db);
int  idcu_sqlite_execute(idcu_SQLiteDB* db, const char* sql);
int  idcu_sqlite_query(idcu_SQLiteDB* db, const char* sql, idcu_Vector* results);
int  idcu_sqlite_prepare(idcu_SQLiteDB* db, const char* sql, void** stmt);
int  idcu_sqlite_bind_text(void* stmt, int index, const char* value);
int  idcu_sqlite_bind_int(void* stmt, int index, int64_t value);
int  idcu_sqlite_step(void* stmt);
const char* idcu_sqlite_column_text(void* stmt, int index);
int64_t idcu_sqlite_column_int(void* stmt, int index);
int  idcu_sqlite_finalize(void* stmt);

// 事务
typedef struct idcu_Transaction {
    idcu_SQLiteDB* db;
    int active;
} idcu_Transaction;

int  idcu_sqlite_begin_transaction(idcu_SQLiteDB* db);
int  idcu_sqlite_commit(idcu_SQLiteDB* db);
int  idcu_sqlite_rollback(idcu_SQLiteDB* db);
```

### 2.4 跨平台适配
- **SQLite**: 使用系统 SQLite3 或嵌入式 amalgamation
- **文件路径**: 统一处理 Windows 和 Linux 路径分隔符
- **互斥锁**: 使用 idcu-common 中的跨平台实现

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] KV 存储可以正常读写各种类型
- [ ] KV 存储持久化正确（重启后数据不丢失）
- [ ] SQLite 数据库可以创建表和插入数据
- [ ] SQLite 可以正确查询结果
- [ ] 预处理语句和参数绑定正常工作
- [ ] 事务可以正常提交和回滚
- [ ] 多线程并发操作安全（无崩溃，数据一致）
- [ ] 所有 API 正确处理 NULL 指针

### 3.2 性能验收
- KV 存储读写延迟 ≤ 1ms（平均）
- SQLite 查询延迟 ≤ 10ms（简单查询）
- KV 存储支持 10000+ 键值对
- SQLite 插入吞吐量 ≥ 1000 条/秒（批量）
- 内存占用 ≤ 10MB（10000 个键值对）

### 3.3 异常验收
- [ ] 数据库文件不存在时正确创建
- [ ] SQL 语法错误返回明确错误码
- [ ] 事务回滚正确恢复数据
- [ ] 磁盘空间不足时返回错误
- [ ] 传入无效参数返回明确错误码

---

## 4. 执行计划

### 4.1 工期
5 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（45 分钟）
- D1-45: 完成 KV 存储实现（1.5 小时）
- D1-135: 完成 SQLite 封装（1.5 小时）
- D1-225: 完成事务支持（30 分钟）
- D1-255: 完成单元测试（30 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + SQLite + 数据库）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_
- 所有公共 API 有 Doxygen 风格注释

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 测试覆盖 KV 存储各种类型和操作
- 测试覆盖 SQLite 基本操作和事务
- 测试覆盖异常场景

### 5.3 部署指引
- 编译命令：`cmake -B build && cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::storage)`
- 依赖：idcu-common, idcu-json, SQLite3

---

## 6. 风险与应对

### 6.1 风险1
描述：SQLite 锁争用导致性能下降  
应对：提供连接池或读写锁优化

### 6.2 风险2
描述：KV 存储同步时阻塞  
应对：支持异步同步选项

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-storage/include/idcu/storage
mkdir -p libs/idcu-storage/src/idcu/storage
mkdir -p libs/idcu-storage/tests
mkdir -p libs/idcu-storage/examples
```

### 2. 创建头文件和实现
- storage.h: 头文件定义
- kvstore.c: KV 存储实现
- sqlite.c: SQLite 封装
- transaction.c: 事务支持

### 3. 创建 CMakeLists.txt 和 module.yaml

### 4. 创建 README.md

---

## 8. 验证检查清单

- [ ] 存储头文件已创建
- [ ] 存储实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 键值存储可以正常读写
- [ ] SQLite 数据库可以正常操作
- [ ] 事务可以正常提交和回滚
- [ ] 单元测试通过率 100%
- [ ] 性能测试达标
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-storage/
git commit -m "feat: add idcu-storage library

- Add key-value store with persistence
- Add SQLite database wrapper
- Add transaction support
- Add data serialization
- Add query interface
- Add thread-safe operations
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 数据库锁定 | 多线程未正确使用锁 | 确保每个操作正确获取和释放锁 |
| KV 数据丢失 | 未调用 sync | 修改后记得调用 idcu_kvstore_sync() |
| SQLite 错误 | SQL 语法错误 | 检查 SQL 语句语法 |
| 事务回滚失败 | 未正确处理错误 | 检查每个操作的返回码 |
