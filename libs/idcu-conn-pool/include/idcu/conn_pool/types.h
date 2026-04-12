#ifndef IDCU_CONN_POOL_TYPES_H
#define IDCU_CONN_POOL_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* idcu_Connection;

typedef int (*idcu_ConnectionCreate)(idcu_Connection* conn, void* user_data);
typedef void (*idcu_ConnectionDestroy)(idcu_Connection conn, void* user_data);
typedef int (*idcu_ConnectionValidate)(idcu_Connection conn, void* user_data);

typedef struct idcu_PooledConnection {
    idcu_Connection conn;
    uint64_t created_at;
    uint64_t last_used_at;
    int in_use;
    int valid;
    struct idcu_PooledConnection* next;
} idcu_PooledConnection;

typedef struct idcu_ConnectionPoolConfig {
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

typedef struct idcu_ConnectionPoolStats {
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

typedef struct idcu_ConnectionPool {
    idcu_PooledConnection* connections;
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
    
    int cleanup_running;
    uint64_t cleanup_interval_ms;
    
    idcu_ConnectionPoolStats stats;
    int initialized;
} idcu_ConnectionPool;

#ifdef __cplusplus
}
#endif

#endif
