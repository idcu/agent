#include "connection_pool.h"
#include "log.h"
#include "health_check.h"
#include <string.h>
#include <stdio.h>

static uint64_t get_current_time_ms(void)
{
    return idcu_health_get_uptime_ms();
}

int idcu_connection_pool_init(idcu_ConnectionPool* pool, const char* name, int protocol)
{
    if (!pool || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(pool, 0, sizeof(idcu_ConnectionPool));
    strncpy(pool->pool_name, name, sizeof(pool->pool_name) - 1);
    pool->protocol = protocol;
    pool->idle_timeout_ms = IDCU_POOL_CONN_IDLE_TIMEOUT_MS;
    pool->max_retries = IDCU_POOL_CONN_MAX_RETRIES;
    pool->initialized = 0;
    
    int ret = idcu_mutex_init(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("connection pool %s: failed to init lock, code=%d", name, ret);
        return ret;
    }
    
    pool->initialized = 1;
    IDCU_LOG_INFO("connection pool %s initialized (protocol=%s)", 
                  name, protocol == IDCU_NET_PROTO_TCP ? "TCP" : "UDP");
    return IDCU_ERR_SUCCESS;
}

void idcu_connection_pool_destroy(idcu_ConnectionPool* pool)
{
    if (!pool || !pool->initialized) {
        return;
    }
    
    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("connection pool %s: failed to lock during destroy", pool->pool_name);
        return;
    }
    
    for (uint32_t i = 0; i < pool->count; i++) {
        idcu_PooledConnection* conn = &pool->connections[i];
        if (conn->sock.fd != IDCU_INVALID_SOCKET) {
            idcu_network_socket_close(&conn->sock);
            idcu_network_socket_destroy(&conn->sock);
        }
    }
    
    pool->count = 0;
    pool->initialized = 0;
    
    idcu_mutex_unlock(&pool->lock);
    idcu_mutex_destroy(&pool->lock);
    
    IDCU_LOG_INFO("connection pool %s destroyed", pool->pool_name);
}

static int is_connection_valid(idcu_PooledConnection* conn)
{
    if (!conn) {
        return 0;
    }
    if (conn->sock.fd == IDCU_INVALID_SOCKET) {
        return 0;
    }
    if (!conn->sock.connected) {
        return 0;
    }
    return 1;
}

static idcu_PooledConnection* find_idle_connection(idcu_ConnectionPool* pool, 
                                                     const char* addr, uint16_t port)
{
    for (uint32_t i = 0; i < pool->count; i++) {
        idcu_PooledConnection* conn = &pool->connections[i];
        if (!conn->in_use && 
            strcmp(conn->remote_addr, addr) == 0 && 
            conn->remote_port == port &&
            is_connection_valid(conn)) {
            return conn;
        }
    }
    return NULL;
}

static idcu_PooledConnection* create_new_connection(idcu_ConnectionPool* pool, 
                                                      const char* addr, uint16_t port)
{
    if (pool->count >= IDCU_POOL_MAX_CONNECTIONS) {
        IDCU_LOG_WARN("connection pool %s: max connections reached", pool->pool_name);
        return NULL;
    }
    
    idcu_PooledConnection* conn = &pool->connections[pool->count];
    memset(conn, 0, sizeof(idcu_PooledConnection));
    
    int ret = idcu_network_socket_create(&conn->sock, pool->protocol);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("connection pool %s: failed to create socket", pool->pool_name);
        return NULL;
    }
    
    for (int retry = 0; retry < pool->max_retries; retry++) {
        ret = idcu_network_socket_connect(&conn->sock, addr, port);
        if (ret == IDCU_ERR_SUCCESS) {
            break;
        }
        IDCU_LOG_WARN("connection pool %s: connect attempt %d failed, retrying...", 
                      pool->pool_name, retry + 1);
    }
    
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("connection pool %s: failed to connect to %s:%d after %d retries",
                      pool->pool_name, addr, port, pool->max_retries);
        idcu_network_socket_destroy(&conn->sock);
        return NULL;
    }
    
    strncpy(conn->remote_addr, addr, sizeof(conn->remote_addr) - 1);
    conn->remote_port = port;
    conn->protocol = pool->protocol;
    conn->in_use = 0;
    conn->last_used_ms = get_current_time_ms();
    conn->ref_count = 0;
    
    pool->count++;
    IDCU_LOG_DEBUG("connection pool %s: created new connection to %s:%d (total=%u)",
                   pool->pool_name, addr, port, pool->count);
    return conn;
}

int idcu_connection_pool_acquire(idcu_ConnectionPool* pool, const char* addr, uint16_t port, 
                                  idcu_ConnectionHandle* handle)
{
    if (!pool || !pool->initialized || !addr || !handle) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    idcu_PooledConnection* conn = find_idle_connection(pool, addr, port);
    
    if (!conn) {
        conn = create_new_connection(pool, addr, port);
        if (!conn) {
            idcu_mutex_unlock(&pool->lock);
            return IDCU_ERR_GENERAL;
        }
    }
    
    conn->in_use = 1;
    conn->ref_count++;
    conn->last_used_ms = get_current_time_ms();
    
    handle->pool = pool;
    handle->conn = conn;
    handle->acquired = 1;
    
    idcu_mutex_unlock(&pool->lock);
    
    IDCU_LOG_DEBUG("connection pool %s: acquired connection to %s:%d (ref=%d)",
                   pool->pool_name, addr, port, conn->ref_count);
    return IDCU_ERR_SUCCESS;
}

int idcu_connection_pool_release(idcu_ConnectionHandle* handle)
{
    if (!handle || !handle->acquired || !handle->pool || !handle->conn) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_ConnectionPool* pool = handle->pool;
    idcu_PooledConnection* conn = handle->conn;
    
    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    conn->in_use = 0;
    conn->last_used_ms = get_current_time_ms();
    
    handle->acquired = 0;
    handle->pool = NULL;
    handle->conn = NULL;
    
    idcu_mutex_unlock(&pool->lock);
    
    IDCU_LOG_DEBUG("connection pool %s: released connection to %s:%d",
                   pool->pool_name, conn->remote_addr, conn->remote_port);
    return IDCU_ERR_SUCCESS;
}

int idcu_connection_pool_send(idcu_ConnectionHandle* handle, const void* data, size_t len, size_t* sent)
{
    if (!handle || !handle->acquired || !handle->conn) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_PooledConnection* conn = handle->conn;
    if (!is_connection_valid(conn)) {
        IDCU_LOG_ERROR("connection pool: invalid connection for send");
        return IDCU_ERR_NETWORK_CONNECT;
    }
    
    return idcu_network_socket_send(&conn->sock, data, len, sent);
}

int idcu_connection_pool_recv(idcu_ConnectionHandle* handle, void* data, size_t len, size_t* received)
{
    if (!handle || !handle->acquired || !handle->conn) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    idcu_PooledConnection* conn = handle->conn;
    if (!is_connection_valid(conn)) {
        IDCU_LOG_ERROR("connection pool: invalid connection for recv");
        return IDCU_ERR_NETWORK_CONNECT;
    }
    
    return idcu_network_socket_recv(&conn->sock, data, len, received);
}

void idcu_connection_pool_cleanup_idle(idcu_ConnectionPool* pool)
{
    if (!pool || !pool->initialized) {
        return;
    }
    
    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return;
    }
    
    uint64_t now = get_current_time_ms();
    uint32_t removed = 0;
    
    for (uint32_t i = 0; i < pool->count; ) {
        idcu_PooledConnection* conn = &pool->connections[i];
        if (!conn->in_use && (now - conn->last_used_ms) > pool->idle_timeout_ms) {
            IDCU_LOG_DEBUG("connection pool %s: cleaning up idle connection to %s:%d",
                           pool->pool_name, conn->remote_addr, conn->remote_port);
            
            if (conn->sock.fd != IDCU_INVALID_SOCKET) {
                idcu_network_socket_close(&conn->sock);
                idcu_network_socket_destroy(&conn->sock);
            }
            
            if (i < pool->count - 1) {
                memmove(&pool->connections[i], &pool->connections[i + 1],
                        (pool->count - i - 1) * sizeof(idcu_PooledConnection));
            }
            pool->count--;
            removed++;
        } else {
            i++;
        }
    }
    
    idcu_mutex_unlock(&pool->lock);
    
    if (removed > 0) {
        IDCU_LOG_DEBUG("connection pool %s: cleaned up %u idle connections", pool->pool_name, removed);
    }
}

int idcu_connection_pool_get_stats(idcu_ConnectionPool* pool, uint32_t* total, uint32_t* in_use, uint32_t* idle)
{
    if (!pool || !pool->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = idcu_mutex_lock(&pool->lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    
    if (total) *total = pool->count;
    if (in_use || idle) {
        uint32_t use_cnt = 0;
        for (uint32_t i = 0; i < pool->count; i++) {
            if (pool->connections[i].in_use) {
                use_cnt++;
            }
        }
        if (in_use) *in_use = use_cnt;
        if (idle) *idle = pool->count - use_cnt;
    }
    
    idcu_mutex_unlock(&pool->lock);
    return IDCU_ERR_SUCCESS;
}
