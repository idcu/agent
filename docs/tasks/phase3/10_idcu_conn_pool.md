# 任务 3.10: idcu-conn-pool - 连接池库

## 目标

创建通用连接池库，支持：
- 连接池管理
- 连接复用
- 连接创建和销毁
- 连接获取和释放
- 连接健康检查
- 空闲连接清理
- 配置参数（最小/最大连接数等）
- 线程安全

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-conn-pool/include/idcu/conn_pool
mkdir -p libs/idcu-conn-pool/src/idcu/conn_pool
mkdir -p libs/idcu-conn-pool/tests
mkdir -p libs/idcu-conn-pool/examples
```

### 2. 创建连接池头文件 (conn_pool.h)

创建 `libs/idcu-conn-pool/include/idcu/conn_pool/conn_pool.h`：

```c
#ifndef IDCU_CONN_POOL_CONN_POOL_H
#define IDCU_CONN_POOL_CONN_POOL_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/lock.h"
#include "idcu/common/condition.h"
#include "idcu/common/thread.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

int idcu_conn_pool_config_init(idcu_ConnectionPoolConfig* config);

int  idcu_conn_pool_init(idcu_ConnectionPool* pool, const idcu_ConnectionPoolConfig* config,
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

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-conn-pool/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-conn-pool VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-conn-pool STATIC
    src/idcu/conn_pool/conn_pool.c
)

target_include_directories(idcu-conn-pool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-conn-pool PRIVATE
    idcu::common
    idcu::log
)

add_library(idcu::conn-pool ALIAS idcu-conn-pool)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-conn-pool/module.yaml`：

```yaml
name: idcu-conn-pool
version: 1.0.0
description: Connection pool library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-conn-pool

headers:
  - idcu/conn_pool/conn_pool.h

features:
  - pool: Generic connection pool
  - reuse: Connection reuse
  - create_destroy: Connection create/destroy callbacks
  - acquire_release: Connection acquire/release
  - health_check: Connection health check
  - cleanup: Idle connection cleanup
  - config: Configurable parameters (min/max connections)
  - thread_safe: Thread-safe operations
  - stats: Pool statistics

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-conn-pool/README.md`：

```markdown
# idcu-conn-pool

IDCU Agent 的通用连接池库。

## 功能特性

- **通用连接池**: 通用的连接池实现
- **连接复用**: 连接复用提高性能
- **创建/销毁回调**: 连接创建和销毁回调
- **获取/释放**: 连接获取和释放
- **健康检查**: 连接健康检查
- **空闲清理**: 空闲连接清理
- **配置参数**: 可配置参数（最小/最大连接数等）
- **线程安全**: 线程安全操作
- **统计信息**: 连接池统计

## 快速开始

### 定义连接回调

```c
#include "idcu/conn_pool/conn_pool.h"
#include "idcu/network/network.h"

typedef struct
{
    idcu_TcpSocket socket;
    char host[256];
    uint16_t port;
} TcpConnection;

int tcp_connection_create(idcu_Connection* conn, void* user_data)
{
    TcpConnection* tcp_conn = (TcpConnection*)malloc(sizeof(TcpConnection));
    if (!tcp_conn) return IDCU_ERR_OUT_OF_MEMORY;
    
    idcu_tcp_socket_init(&tcp_conn->socket);
    strncpy(tcp_conn->host, "localhost", sizeof(tcp_conn->host));
    tcp_conn->port = 8080;
    
    idcu_NetAddress addr;
    idcu_net_address_init(&addr, IDCU_NET_ADDR_IPV4, tcp_conn->host, tcp_conn->port);
    
    int ret = idcu_tcp_socket_connect(&tcp_conn->socket, &addr);
    if (ret != IDCU_ERR_OK) {
        free(tcp_conn);
        return ret;
    }
    
    *conn = tcp_conn;
    return IDCU_ERR_OK;
}

void tcp_connection_destroy(idcu_Connection conn, void* user_data)
{
    if (!conn) return;
    
    TcpConnection* tcp_conn = (TcpConnection*)conn;
    idcu_tcp_socket_disconnect(&tcp_conn->socket);
    idcu_tcp_socket_destroy(&tcp_conn->socket);
    free(tcp_conn);
}

int tcp_connection_validate(idcu_Connection conn, void* user_data)
{
    if (!conn) return 0;
    
    TcpConnection* tcp_conn = (TcpConnection*)conn;
    return tcp_conn->socket.is_connected ? 1 : 0;
}
```

### 配置连接池

```c
idcu_ConnectionPoolConfig config;
idcu_conn_pool_config_init(&config);

config.min_connections = 5;
config.max_connections = 50;
config.max_idle_time_ms = 30000;
config.max_lifetime_ms = 3600000;
config.connect_timeout_ms = 5000;
config.acquire_timeout_ms = 10000;
config.validate_on_borrow = 1;
config.cleanup_interval_ms = 60000;
```

### 初始化连接池

```c
idcu_ConnectionPool pool;
idcu_conn_pool_init(&pool, &config,
                    tcp_connection_create,
                    tcp_connection_destroy,
                    tcp_connection_validate,
                    NULL);
```

### 获取连接

```c
idcu_Connection conn;
int ret = idcu_conn_pool_acquire(&pool, &conn);
if (ret == IDCU_ERR_OK) {
    TcpConnection* tcp_conn = (TcpConnection*)conn;
    
    const char* data = "Hello!";
    idcu_tcp_socket_send_all(&tcp_conn->socket, data, strlen(data));
    
    idcu_conn_pool_release(&pool, conn);
}
```

### 释放连接

```c
idcu_conn_pool_release(&pool, conn);
```

### 销毁连接池

```c
idcu_conn_pool_destroy(&pool);
```

### 获取统计信息

```c
idcu_ConnectionPoolStats stats;
idcu_conn_pool_get_stats(&pool, &stats);

printf("Total: %zu\n", stats.total_connections);
printf("Idle: %zu\n", stats.idle_connections);
printf("In use: %zu\n", stats.in_use_connections);
printf("Total acquired: %" PRIu64 "\n", stats.total_acquired);
```

## 配置参数

| 参数 | 说明 | 默认值 |
|-----|------|--------|
| min_connections | 最小连接数 | 5 |
| max_connections | 最大连接数 | 50 |
| max_idle_time_ms | 最大空闲时间(ms) | 30000 |
| max_lifetime_ms | 最大生命周期(ms) | 3600000 |
| connect_timeout_ms | 连接超时(ms) | 5000 |
| acquire_timeout_ms | 获取超时(ms) | 10000 |
| validate_on_borrow | 获取时验证 | 1 |
| validate_on_return | 归还时验证 | 0 |
| cleanup_interval_ms | 清理间隔(ms) | 60000 |

## API 文档

详见 [include/idcu/conn_pool/conn_pool.h](include/idcu/conn_pool/conn_pool.h)
```

## 验证检查清单

- [ ] 连接池头文件已创建
- [ ] 连接池实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以创建和销毁连接池
- [ ] 可以获取和释放连接
- [ ] 统计功能正常工作

## Git 提交

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

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 获取连接超时 | 连接池已满 | 增加 max_connections 或减少连接占用时间 |
| 连接验证失败 | 连接已断开 | 实现 validate_func 检查连接状态 |
| 连接泄漏 | 未释放连接 | 确保所有获取的连接都被释放 |
