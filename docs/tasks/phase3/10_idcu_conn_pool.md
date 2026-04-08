# 任务 3.10: idcu-conn-pool - 连接池库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建通用连接池库，提供连接池管理、连接复用、健康检查、空闲清理等功能，支持最小连接数 5、最大连接数 50、连接获取延迟 ≤ 5ms、支持 1000+ 并发连接获取、连接复用率 ≥ 90% 的性能要求。

### 1.2 不做什么
- 不实现具体的连接建立逻辑（由用户通过回调提供）
- 不实现连接协议（TCP/UDP 等由网络库处理）
- 不实现负载均衡
- 不实现连接认证

### 1.3 输入
- 连接池配置参数（min/max 连接数、超时时间等）
- 连接创建/销毁/验证回调函数
- 连接获取请求
- 连接释放请求

### 1.4 输出
- 连接池句柄
- 连接对象
- 连接池统计信息
- 错误码：0 表示成功，非 0 表示错误

### 1.5 前置依赖
- idcu-common 基础库已可用
- idcu-log 日志库已可用
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **数据结构**: 使用 Vector 管理连接列表
- **同步原语**: Mutex 保证线程安全、Condition 实现等待机制
- **清理机制**: 后台线程定期清理空闲连接
- **健康检查**: 回调函数由用户提供
- **连接复用**: LIFO 策略优先复用最近使用的连接

### 2.2 核心逻辑
```
连接池初始化：
1. 初始化配置和回调
2. 创建初始连接（min_connections）
3. 启动后台清理线程

连接获取流程：
1. 加锁
2. 查找空闲有效连接
3. 有则直接返回
4. 无且未达最大连接数则创建新连接
5. 无且已满则等待直到有连接释放或超时
6. 验证连接（如配置了 validate_on_borrow）
7. 标记为使用中
8. 解锁返回

连接释放流程：
1. 加锁
2. 验证连接（如配置了 validate_on_return）
3. 有效则放回空闲队列
4. 无效则销毁
5. 通知等待者
6. 解锁

后台清理流程：
1. 定期唤醒（cleanup_interval_ms）
2. 检查空闲连接
3. 销毁超过 max_idle_time_ms 的连接
4. 销毁超过 max_lifetime_ms 的连接
5. 保持至少 min_connections 个连接
```

### 2.3 数据结构/接口
```c
// 主要头文件：idcu/conn_pool/conn_pool.h

typedef void* idcu_Connection;

typedef int (*idcu_ConnectionCreate)(idcu_Connection* conn, void* user_data);
typedef void (*idcu_ConnectionDestroy)(idcu_Connection conn, void* user_data);
typedef int (*idcu_ConnectionValidate)(idcu_Connection conn, void* user_data);

typedef struct
{
    idcu_Connection conn;
    uint64_t created_at;
    uint64_t last_used_at;
    int in_use;
    int valid;
} idcu_PooledConnection;

typedef struct
{
    idcu_Vector connections;
    idcu_Mutex lock;
    idcu_Condition not_empty;
    idcu_Condition not_full;
    
    idcu_ConnectionCreate create_func;
    idcu_ConnectionDestroy destroy_func;
    idcu_ConnectionValidate validate_func;
    void* user_data;
    
    size_t min_connections;
    size_t max_connections;
    size_t current_connections;
    size_t idle_connections;
    size_t in_use_connections;
    
    uint64_t max_idle_time_ms;
    uint64_t max_lifetime_ms;
    uint64_t connect_timeout_ms;
    uint64_t acquire_timeout_ms;
    
    int validate_on_borrow;
    int validate_on_return;
    int validate_periodically;
    
    idcu_Thread cleanup_thread;
    int cleanup_running;
    uint64_t cleanup_interval_ms;
    
    int initialized;
} idcu_ConnectionPool;

typedef struct
{
    size_t min_connections;
    size_t max_connections;
    uint64_t max_idle_time_ms;
    uint64_t max_lifetime_ms;
    uint64_t connect_timeout_ms;
    uint64_t acquire_timeout_ms;
    int validate_on_borrow;
    int validate_on_return;
    int validate_periodically;
    uint64_t cleanup_interval_ms;
} idcu_ConnectionPoolConfig;

// 核心 API
int idcu_conn_pool_config_init(idcu_ConnectionPoolConfig* config);
int idcu_conn_pool_init(idcu_ConnectionPool* pool, const idcu_ConnectionPoolConfig* config,
                        idcu_ConnectionCreate create_func, idcu_ConnectionDestroy destroy_func,
                        idcu_ConnectionValidate validate_func, void* user_data);
void idcu_conn_pool_destroy(idcu_ConnectionPool* pool);

int idcu_conn_pool_acquire(idcu_ConnectionPool* pool, idcu_Connection* conn);
int idcu_conn_pool_acquire_timeout(idcu_ConnectionPool* pool, idcu_Connection* conn, uint64_t timeout_ms);
int idcu_conn_pool_release(idcu_ConnectionPool* pool, idcu_Connection conn);
int idcu_conn_pool_invalidate(idcu_ConnectionPool* pool, idcu_Connection conn);

size_t idcu_conn_pool_get_total_connections(const idcu_ConnectionPool* pool);
size_t idcu_conn_pool_get_idle_connections(const idcu_ConnectionPool* pool);
size_t idcu_conn_pool_get_in_use_connections(const idcu_ConnectionPool* pool);

int idcu_conn_pool_validate_all(idcu_ConnectionPool* pool);
int idcu_conn_pool_cleanup(idcu_ConnectionPool* pool);
int idcu_conn_pool_clear(idcu_ConnectionPool* pool);

typedef struct
{
    size_t total_connections;
    size_t idle_connections;
    size_t in_use_connections;
    size_t min_connections;
    size_t max_connections;
    uint64_t total_acquired;
    uint64_t total_released;
    uint64_t total_created;
    uint64_t total_destroyed;
    uint64_t total_validation_failed;
} idcu_ConnectionPoolStats;

void idcu_conn_pool_get_stats(const idcu_ConnectionPool* pool, idcu_ConnectionPoolStats* stats);
void idcu_conn_pool_reset_stats(idcu_ConnectionPool* pool);
```

### 2.4 跨平台适配
- **Windows**: 使用 Win32 线程 API（CreateThread、WaitForSingleObject）
- **Linux/macOS**: 使用 pthread API
- **统一接口**: 封装在 idcu-common 中，通过 idcu_Thread、idcu_Mutex 等统一调用

---

## 3. 验收标准（可量化）

### 3.1 功能验收
- [ ] 可以创建和销毁连接池
- [ ] 可以获取和释放连接
- [ ] 空闲连接正确复用
- [ ] 连接数在 min/max 之间波动
- [ ] 超时获取连接返回正确错误
- [ ] 连接健康检查正常工作
- [ ] 后台清理线程正常清理空闲连接
- [ ] 连接池统计信息正确
- [ ] 多线程并发安全
- [ ] 跨平台正常运行（Windows + Linux）

### 3.2 性能验收
- 连接获取延迟 ≤ 5ms（空闲连接）
- 连接获取延迟 ≤ 50ms（需要新建连接）
- 支持 1000+ 并发连接获取
- 连接复用率 ≥ 90%
- 内存占用 ≤ 5MB（50 连接）
- 后台清理对性能影响 ≤ 5%

### 3.3 异常验收
- [ ] 连接创建失败返回正确错误码
- [ ] 连接验证失败时正确处理
- [ ] 连接池满时正确等待或超时
- [ ] NULL 指针检查正确
- [ ] 清理线程退出时资源正确释放

---

## 4. 执行计划

### 4.1 工期
4 小时/人

### 4.2 里程碑
- D1-00: 完成头文件定义和数据结构（45 分钟）
- D1-45: 完成核心连接池逻辑（1 小时）
- D1-105: 完成连接获取/释放/验证（45 分钟）
- D1-150: 完成后台清理和统计（30 分钟）
- D1-180: 完成单元测试（60 分钟）

### 4.3 人力
1 人（技能要求：C 语言 + 多线程编程）

---

## 5. 工程化要求

### 5.1 编码规范
- 对齐项目 .clang-format 规范
- 函数名小写 + 下划线，前缀 idcu_
- 结构体前缀 idcu_

### 5.2 测试要求
- 单元测试覆盖率 ≥ 80%
- 并发测试覆盖 100+ 线程
- 跨平台测试（Windows + Linux）

### 5.3 部署指引
- 编译命令：`cmake -B build &amp;&amp; cmake --build build`
- 链接：`target_link_libraries(myapp PRIVATE idcu::conn-pool)`

---

## 6. 风险与应对

### 6.1 风险1
描述：高并发下锁竞争导致性能下降  
应对：使用细粒度锁或读写锁，优化临界区

### 6.2 风险2
描述：连接验证耗时过长影响获取速度  
应对：允许异步验证，或在后台线程进行验证

---

## 7. 详细实现步骤

### 1. 创建目录结构
```bash
mkdir -p libs/idcu-conn-pool/include/idcu/conn_pool
mkdir -p libs/idcu-conn-pool/src/idcu/conn_pool
mkdir -p libs/idcu-conn-pool/tests
mkdir -p libs/idcu-conn-pool/examples
```

### 2. 创建连接池头文件 (conn_pool.h)

### 3. 创建 CMakeLists.txt

### 4. 创建模块配置文件 (module.yaml)

### 5. 创建 README.md

---

## 8. 验证检查清单

- [ ] 连接池头文件已创建
- [ ] 连接池实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以创建和销毁连接池
- [ ] 可以获取和释放连接
- [ ] 统计功能正常工作
- [ ] 单元测试通过
- [ ] 跨平台测试通过
- [ ] 已提交 Git

---

## 9. Git 提交

```bash
git add libs/idcu-conn-pool/
git commit -m "feat: add idcu-conn-pool library

- Add generic connection pool
- Add connection reuse
- Add connection create/destroy callbacks
- Add connection acquire/release
- Add connection health check
- Add idle connection cleanup
- Add configurable parameters (min/max connections)
- Add thread-safe operations
- Add pool statistics
- Add CMake build configuration
- Add module.yaml metadata"
```

---

## 10. 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 获取连接超时 | 连接池已满 | 增加 max_connections 或减少连接占用时间 |
| 连接验证失败 | 连接已断开 | 实现 validate_func 检查连接状态 |
| 连接泄漏 | 未释放连接 | 确保所有获取的连接都被释放 |
| 性能下降 | 锁竞争 | 优化临界区，考虑使用读写锁 |
