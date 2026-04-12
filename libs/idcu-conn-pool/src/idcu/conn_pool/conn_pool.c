#include <idcu/conn_pool/conn_pool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

static uint64_t get_timestamp_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

int idcu_conn_pool_config_init(idcu_ConnectionPoolConfig* config) {
    if (!config) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(config, 0, sizeof(*config));
    config->min_connections = 5;
    config->max_connections = 50;
    config->max_idle_time_ms = 300000;
    config->max_lifetime_ms = 3600000;
    config->connect_timeout_ms = 30000;
    config->acquire_timeout_ms = 30000;
    config->validate_on_borrow = 0;
    config->validate_on_return = 0;
    config->validate_periodically = 0;
    config->cleanup_interval_ms = 60000;
    
    return IDCU_ERR_OK;
}

int idcu_conn_pool_init(idcu_ConnectionPool* pool, const idcu_ConnectionPoolConfig* config,
                        idcu_ConnectionCreate create_func, idcu_ConnectionDestroy destroy_func,
                        idcu_ConnectionValidate validate_func, void* user_data) {
    if (!pool || !config || !create_func || !destroy_func) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(pool, 0, sizeof(*pool));
    
    pool->min_connections = config->min_connections;
    pool->max_connections = config->max_connections;
    pool->max_idle_time_ms = config->max_idle_time_ms;
    pool->max_lifetime_ms = config->max_lifetime_ms;
    pool->connect_timeout_ms = config->connect_timeout_ms;
    pool->acquire_timeout_ms = config->acquire_timeout_ms;
    pool->validate_on_borrow = config->validate_on_borrow;
    pool->validate_on_return = config->validate_on_return;
    pool->validate_periodically = config->validate_periodically;
    pool->cleanup_interval_ms = config->cleanup_interval_ms;
    
    pool->create_func = create_func;
    pool->destroy_func = destroy_func;
    pool->validate_func = validate_func;
    pool->user_data = user_data;
    
    idcu_mutex_init(&pool->lock);
    idcu_cond_init(&pool->not_empty);
    idcu_cond_init(&pool->not_full);
    
    pool->stats.min_connections = pool->min_connections;
    pool->stats.max_connections = pool->max_connections;
    
    for (size_t i = 0; i < pool->min_connections; i++) {
        idcu_Connection conn;
        if (pool->create_func(&conn, pool->user_data) == IDCU_ERR_OK) {
            idcu_PooledConnection* pooled = (idcu_PooledConnection*)calloc(1, sizeof(idcu_PooledConnection));
            if (pooled) {
                pooled->conn = conn;
                pooled->created_at = get_timestamp_ms();
                pooled->last_used_at = pooled->created_at;
                pooled->valid = 1;
                pooled->in_use = 0;
                pooled->next = pool->connections;
                pool->connections = pooled;
                pool->current_connections++;
                pool->idle_connections++;
                pool->stats.total_connections++;
                pool->stats.total_created++;
            }
        }
    }
    
    pool->cleanup_running = 0;
    pool->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_conn_pool_destroy(idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return;
    }
    
    idcu_mutex_lock(&pool->lock);
    
    idcu_PooledConnection* conn = pool->connections;
    while (conn) {
        idcu_PooledConnection* next = conn->next;
        pool->destroy_func(conn->conn, pool->user_data);
        pool->stats.total_destroyed++;
        free(conn);
        conn = next;
    }
    
    pool->connections = NULL;
    pool->current_connections = 0;
    pool->idle_connections = 0;
    pool->in_use_connections = 0;
    
    idcu_Mutex lock_copy = pool->lock;
    pool->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    
    idcu_cond_destroy(&pool->not_empty);
    idcu_cond_destroy(&pool->not_full);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_conn_pool_acquire(idcu_ConnectionPool* pool, idcu_Connection* conn) {
    return idcu_conn_pool_acquire_timeout(pool, conn, pool->acquire_timeout_ms);
}

int idcu_conn_pool_acquire_timeout(idcu_ConnectionPool* pool, idcu_Connection* conn, uint64_t timeout_ms) {
    if (!pool || !pool->initialized || !conn) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&pool->lock);
    
    uint64_t start_time = get_timestamp_ms();
    
    while (1) {
        idcu_PooledConnection* prev = NULL;
        idcu_PooledConnection* current = pool->connections;
        
        while (current) {
            if (!current->in_use && current->valid) {
                if (pool->validate_on_borrow && pool->validate_func) {
                    if (pool->validate_func(current->conn, pool->user_data) != IDCU_ERR_OK) {
                        current->valid = 0;
                        pool->stats.total_validation_failed++;
                        prev = current;
                        current = current->next;
                        continue;
                    }
                }
                
                current->in_use = 1;
                current->last_used_at = get_timestamp_ms();
                pool->idle_connections--;
                pool->in_use_connections++;
                pool->stats.in_use_connections++;
                pool->stats.idle_connections = pool->idle_connections;
                pool->stats.total_acquired++;
                
                *conn = current->conn;
                
                idcu_cond_signal(&pool->not_full);
                idcu_mutex_unlock(&pool->lock);
                return IDCU_ERR_OK;
            }
            
            prev = current;
            current = current->next;
        }
        
        if (pool->current_connections < pool->max_connections) {
            idcu_Connection new_conn;
            if (pool->create_func(&new_conn, pool->user_data) == IDCU_ERR_OK) {
                idcu_PooledConnection* pooled = (idcu_PooledConnection*)calloc(1, sizeof(idcu_PooledConnection));
                if (pooled) {
                    pooled->conn = new_conn;
                    pooled->created_at = get_timestamp_ms();
                    pooled->last_used_at = pooled->created_at;
                    pooled->valid = 1;
                    pooled->in_use = 1;
                    pooled->next = pool->connections;
                    pool->connections = pooled;
                    pool->current_connections++;
                    pool->in_use_connections++;
                    pool->stats.total_connections++;
                    pool->stats.in_use_connections = pool->in_use_connections;
                    pool->stats.total_created++;
                    pool->stats.total_acquired++;
                    
                    *conn = new_conn;
                    
                    idcu_mutex_unlock(&pool->lock);
                    return IDCU_ERR_OK;
                } else {
                    pool->destroy_func(new_conn, pool->user_data);
                }
            }
        }
        
        uint64_t elapsed = get_timestamp_ms() - start_time;
        if (elapsed >= timeout_ms) {
            idcu_mutex_unlock(&pool->lock);
            return IDCU_ERR_TIMEOUT;
        }
        
        idcu_cond_wait(&pool->not_empty, &pool->lock);
    }
}

int idcu_conn_pool_release(idcu_ConnectionPool* pool, idcu_Connection conn) {
    if (!pool || !pool->initialized || !conn) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&pool->lock);
    
    idcu_PooledConnection* current = pool->connections;
    while (current) {
        if (current->conn == conn && current->in_use) {
            if (pool->validate_on_return && pool->validate_func) {
                if (pool->validate_func(current->conn, pool->user_data) != IDCU_ERR_OK) {
                    current->valid = 0;
                    pool->stats.total_validation_failed++;
                }
            }
            
            if (current->valid) {
                current->in_use = 0;
                current->last_used_at = get_timestamp_ms();
                pool->in_use_connections--;
                pool->idle_connections++;
                pool->stats.in_use_connections = pool->in_use_connections;
                pool->stats.idle_connections = pool->idle_connections;
                pool->stats.total_released++;
                
                idcu_cond_signal(&pool->not_empty);
            } else {
                pool->destroy_func(current->conn, pool->user_data);
                pool->stats.total_destroyed++;
                
                idcu_PooledConnection* prev = NULL;
                idcu_PooledConnection* to_remove = pool->connections;
                while (to_remove) {
                    if (to_remove == current) {
                        if (prev) {
                            prev->next = to_remove->next;
                        } else {
                            pool->connections = to_remove->next;
                        }
                        free(to_remove);
                        pool->current_connections--;
                        pool->in_use_connections--;
                        pool->stats.total_connections--;
                        pool->stats.in_use_connections = pool->in_use_connections;
                        break;
                    }
                    prev = to_remove;
                    to_remove = to_remove->next;
                }
            }
            
            idcu_mutex_unlock(&pool->lock);
            return IDCU_ERR_OK;
        }
        current = current->next;
    }
    
    idcu_mutex_unlock(&pool->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_conn_pool_invalidate(idcu_ConnectionPool* pool, idcu_Connection conn) {
    if (!pool || !pool->initialized || !conn) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&pool->lock);
    
    idcu_PooledConnection* prev = NULL;
    idcu_PooledConnection* current = pool->connections;
    
    while (current) {
        if (current->conn == conn) {
            current->valid = 0;
            
            if (current->in_use) {
                pool->destroy_func(current->conn, pool->user_data);
                pool->stats.total_destroyed++;
                
                if (prev) {
                    prev->next = current->next;
                } else {
                    pool->connections = current->next;
                }
                
                free(current);
                pool->current_connections--;
                pool->in_use_connections--;
                pool->stats.total_connections--;
                pool->stats.in_use_connections = pool->in_use_connections;
            }
            
            idcu_mutex_unlock(&pool->lock);
            return IDCU_ERR_OK;
        }
        
        prev = current;
        current = current->next;
    }
    
    idcu_mutex_unlock(&pool->lock);
    return IDCU_ERR_NOT_FOUND;
}

size_t idcu_conn_pool_get_total_connections(const idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return 0;
    }
    return pool->current_connections;
}

size_t idcu_conn_pool_get_idle_connections(const idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return 0;
    }
    return pool->idle_connections;
}

size_t idcu_conn_pool_get_in_use_connections(const idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return 0;
    }
    return pool->in_use_connections;
}

int idcu_conn_pool_validate_all(idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (!pool->validate_func) {
        return IDCU_ERR_OK;
    }
    
    idcu_mutex_lock(&pool->lock);
    
    idcu_PooledConnection* current = pool->connections;
    while (current) {
        if (!current->in_use) {
            if (pool->validate_func(current->conn, pool->user_data) != IDCU_ERR_OK) {
                current->valid = 0;
                pool->stats.total_validation_failed++;
            }
        }
        current = current->next;
    }
    
    idcu_mutex_unlock(&pool->lock);
    return IDCU_ERR_OK;
}

int idcu_conn_pool_cleanup(idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&pool->lock);
    
    uint64_t now = get_timestamp_ms();
    idcu_PooledConnection* prev = NULL;
    idcu_PooledConnection* current = pool->connections;
    
    while (current) {
        idcu_PooledConnection* next = current->next;
        
        int should_remove = 0;
        
        if (!current->in_use && !current->valid) {
            should_remove = 1;
        } else if (!current->in_use && pool->max_idle_time_ms > 0) {
            if (now - current->last_used_at > pool->max_idle_time_ms) {
                if (pool->current_connections > pool->min_connections) {
                    should_remove = 1;
                }
            }
        } else if (!current->in_use && pool->max_lifetime_ms > 0) {
            if (now - current->created_at > pool->max_lifetime_ms) {
                if (pool->current_connections > pool->min_connections) {
                    should_remove = 1;
                }
            }
        }
        
        if (should_remove) {
            pool->destroy_func(current->conn, pool->user_data);
            pool->stats.total_destroyed++;
            
            if (prev) {
                prev->next = next;
            } else {
                pool->connections = next;
            }
            
            free(current);
            pool->current_connections--;
            pool->idle_connections--;
            pool->stats.total_connections--;
            pool->stats.idle_connections = pool->idle_connections;
        } else {
            prev = current;
        }
        
        current = next;
    }
    
    idcu_mutex_unlock(&pool->lock);
    return IDCU_ERR_OK;
}

int idcu_conn_pool_clear(idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&pool->lock);
    
    idcu_PooledConnection* conn = pool->connections;
    while (conn) {
        idcu_PooledConnection* next = conn->next;
        pool->destroy_func(conn->conn, pool->user_data);
        pool->stats.total_destroyed++;
        free(conn);
        conn = next;
    }
    
    pool->connections = NULL;
    pool->current_connections = 0;
    pool->idle_connections = 0;
    pool->in_use_connections = 0;
    pool->stats.total_connections = 0;
    pool->stats.idle_connections = 0;
    pool->stats.in_use_connections = 0;
    
    idcu_mutex_unlock(&pool->lock);
    return IDCU_ERR_OK;
}

void idcu_conn_pool_get_stats(const idcu_ConnectionPool* pool, idcu_ConnectionPoolStats* stats) {
    if (!pool || !stats) {
        return;
    }
    
    idcu_mutex_lock((idcu_Mutex*)&pool->lock);
    *stats = pool->stats;
    stats->total_connections = pool->current_connections;
    stats->idle_connections = pool->idle_connections;
    stats->in_use_connections = pool->in_use_connections;
    idcu_mutex_unlock((idcu_Mutex*)&pool->lock);
}

void idcu_conn_pool_reset_stats(idcu_ConnectionPool* pool) {
    if (!pool || !pool->initialized) {
        return;
    }
    
    idcu_mutex_lock(&pool->lock);
    pool->stats.total_acquired = 0;
    pool->stats.total_released = 0;
    pool->stats.total_created = 0;
    pool->stats.total_destroyed = 0;
    pool->stats.total_validation_failed = 0;
    idcu_mutex_unlock(&pool->lock);
}
