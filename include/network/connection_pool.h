#ifndef IDCU_NETWORK_CONNECTION_POOL_H
#define IDCU_NETWORK_CONNECTION_POOL_H

#include "network/network_layer.h"
#include "common/error_code.h"
#include "common/lock.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_POOL_MAX_CONNECTIONS 32
#define IDCU_POOL_MAX_ADDR_LEN 64
#define IDCU_POOL_CONN_IDLE_TIMEOUT_MS 30000
#define IDCU_POOL_CONN_MAX_RETRIES 3

typedef struct {
    idcu_NetworkSocket sock;
    char remote_addr[IDCU_POOL_MAX_ADDR_LEN];
    uint16_t remote_port;
    int protocol;
    int in_use;
    uint64_t last_used_ms;
    int ref_count;
} idcu_PooledConnection;

typedef struct {
    idcu_PooledConnection connections[IDCU_POOL_MAX_CONNECTIONS];
    uint32_t count;
    char pool_name[64];
    int protocol;
    uint64_t idle_timeout_ms;
    int max_retries;
    idcu_Mutex lock;
    int initialized;
} idcu_ConnectionPool;

typedef struct {
    idcu_ConnectionPool* pool;
    idcu_PooledConnection* conn;
    int acquired;
} idcu_ConnectionHandle;

int idcu_connection_pool_init(idcu_ConnectionPool* pool, const char* name, int protocol);
void idcu_connection_pool_destroy(idcu_ConnectionPool* pool);

int idcu_connection_pool_acquire(idcu_ConnectionPool* pool, const char* addr, uint16_t port, 
                                  idcu_ConnectionHandle* handle);
int idcu_connection_pool_release(idcu_ConnectionHandle* handle);

int idcu_connection_pool_send(idcu_ConnectionHandle* handle, const void* data, size_t len, size_t* sent);
int idcu_connection_pool_recv(idcu_ConnectionHandle* handle, void* data, size_t len, size_t* received);

void idcu_connection_pool_cleanup_idle(idcu_ConnectionPool* pool);
int idcu_connection_pool_get_stats(idcu_ConnectionPool* pool, uint32_t* total, uint32_t* in_use, uint32_t* idle);

#endif
