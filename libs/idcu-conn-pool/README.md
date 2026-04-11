# idcu-conn-pool

连接池库 for IDCU Agent.

## 功能特性

- **连接池管理**: 通用连接池，支持自定义连接创建/销毁/验证回调
- **连接复用**: LIFO策略优先复用最近使用的连接
- **健康检查**: 支持借用时、归还时、定期验证连接
- **空闲清理**: 后台线程定期清理空闲连接
- **线程安全**: Mutex保证线程安全、Condition实现等待机制
- **统计信息**: 连接数统计、获取/释放/创建/销毁计数
- **超时控制**: 连接获取超时、连接创建超时
- **生命周期**: 最小/最大连接数控制
- **连接验证**: 支持自定义验证回调
- **配置灵活**: 完全可配置的连接池参数

## 使用方法

```c
#include <idcu/conn_pool/conn_pool.h>

// 连接创建回调
int my_create_conn(idcu_Connection* conn, void* user_data) {
    // 创建连接
    *conn = /* your_connection_create();
    return IDCU_ERR_OK;
}

// 连接销毁回调
void my_destroy_conn(idcu_Connection conn, void* user_data) {
    your_connection_destroy(conn);
}

// 连接验证回调
int my_validate_conn(idcu_Connection conn, void* user_data) {
    // 验证连接是否有效
    return your_connection_is_valid(conn) ? IDCU_ERR_OK : IDCU_ERR_UNKNOWN;
}

// 初始化连接池配置
idcu_ConnectionPoolConfig config;
idcu_conn_pool_config_init(&config);
config.min_connections = 5;
config.max_connections = 50;
config.max_idle_time_ms = 300000;
config.validate_on_borrow = 1;

// 初始化连接池
idcu_ConnectionPool pool;
idcu_conn_pool_init(&pool, &config, my_create_conn, my_destroy_conn, my_validate_conn, NULL);

// 获取连接
idcu_Connection conn;
if (idcu_conn_pool_acquire(&pool, &conn) == IDCU_ERR_OK) {
    // 使用连接
    use_connection(conn);
    
    // 归还连接
    idcu_conn_pool_release(&pool, conn);
}

// 获取统计信息
idcu_ConnectionPoolStats stats;
idcu_conn_pool_get_stats(&pool, &stats);
printf("Total connections: %zu\n", stats.total_connections);
printf("Idle: %zu\n", stats.idle_connections);
printf("In use: %zu\n", stats.in_use_connections);

// 清理
idcu_conn_pool_destroy(&pool);
```

## API参考

### 配置API
- `idcu_conn_pool_config_init()` - 初始化默认配置

### 核心API
- `idcu_conn_pool_init()` - 初始化连接池
- `idcu_conn_pool_destroy()` - 销毁连接池

### 连接操作
- `idcu_conn_pool_acquire()` - 获取连接
- `idcu_conn_pool_acquire_timeout()` - 带超时获取连接
- `idcu_conn_pool_release()` - 归还连接
- `idcu_conn_pool_invalidate()` - 使连接无效

### 状态查询
- `idcu_conn_pool_get_total_connections()` - 获取总连接数
- `idcu_conn_pool_get_idle_connections()` - 获取空闲连接数
- `idcu_conn_pool_get_in_use_connections()` - 获取使用中连接数

### 管理操作
- `idcu_conn_pool_validate_all()` - 验证所有连接
- `idcu_conn_pool_cleanup()` - 清理空闲连接
- `idcu_conn_pool_clear()` - 清空连接池

### 统计信息
- `idcu_conn_pool_get_stats()` - 获取统计信息
- `idcu_conn_pool_reset_stats()` - 重置统计信息

## 性能指标

- **最小连接数: 5
- **最大连接数: 50
- **连接获取延迟**: ≤ 5ms（空闲连接）
- **连接获取延迟**: ≤ 50ms（需要新建连接）
- **支持并发**: 1000+ 并发连接获取
- **连接复用率**: ≥ 90%
- **内存占用**: ≤ 5MB（50 连接）
- **后台清理对性能影响**: ≤ 5%

## 配置建议

```c
// 小容量连接池
idcu_ConnectionPoolConfig small = {
    .min_connections = 2,
    .max_connections = 10,
    .max_idle_time_ms = 60000,
    .validate_on_borrow = 1,
    .cleanup_interval_ms = 30000
};

// 中等容量连接池
idcu_ConnectionPoolConfig medium = {
    .min_connections = 5,
    .max_connections = 50,
    .max_idle_time_ms = 300000,
    .validate_on_borrow = 1,
    .cleanup_interval_ms = 60000
};

// 大容量连接池
idcu_ConnectionPoolConfig large = {
    .min_connections = 20,
    .max_connections = 200,
    .max_idle_time_ms = 600000,
    .validate_on_borrow = 1,
    .cleanup_interval_ms = 120000
};
```

## 统计信息

```c
typedef struct idcu_ConnectionPoolStats {
    size_t total_connections;     // 总连接数
    size_t idle_connections;      // 空闲连接数
    size_t in_use_connections;    // 使用中连接数
    size_t min_connections;       // 最小连接数
    size_t max_connections;       // 最大连接数
    uint64_t total_acquired;      // 总获取次数
    uint64_t total_released;     // 总归还次数
    uint64_t total_created;      // 总创建次数
    uint64_t total_destroyed;   // 总销毁次数
    uint64_t total_validation_failed; // 验证失败次数
} idcu_ConnectionPoolStats;
```

## 依赖

- idcu-common

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

MIT
