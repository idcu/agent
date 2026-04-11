#ifndef IDCU_CONN_POOL_CONN_POOL_H
#define IDCU_CONN_POOL_CONN_POOL_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/conn_pool/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_conn_pool_config_init(idcu_ConnectionPoolConfig* config);
int  idcu_conn_pool_init(idcu_ConnectionPool* pool, const idcu_ConnectionPoolConfig* config,
                        idcu_ConnectionCreate create_func, idcu_ConnectionDestroy destroy_func,
                        idcu_ConnectionValidate validate_func, void* user_data);
void idcu_conn_pool_destroy(idcu_ConnectionPool* pool);

int  idcu_conn_pool_acquire(idcu_ConnectionPool* pool, idcu_Connection* conn);
int  idcu_conn_pool_acquire_timeout(idcu_ConnectionPool* pool, idcu_Connection* conn, uint64_t timeout_ms);
int  idcu_conn_pool_release(idcu_ConnectionPool* pool, idcu_Connection conn);
int  idcu_conn_pool_invalidate(idcu_ConnectionPool* pool, idcu_Connection conn);

size_t idcu_conn_pool_get_total_connections(const idcu_ConnectionPool* pool);
size_t idcu_conn_pool_get_idle_connections(const idcu_ConnectionPool* pool);
size_t idcu_conn_pool_get_in_use_connections(const idcu_ConnectionPool* pool);

int  idcu_conn_pool_validate_all(idcu_ConnectionPool* pool);
int  idcu_conn_pool_cleanup(idcu_ConnectionPool* pool);
int  idcu_conn_pool_clear(idcu_ConnectionPool* pool);

void idcu_conn_pool_get_stats(const idcu_ConnectionPool* pool, idcu_ConnectionPoolStats* stats);
void idcu_conn_pool_reset_stats(idcu_ConnectionPool* pool);

#ifdef __cplusplus
}
#endif

#endif
