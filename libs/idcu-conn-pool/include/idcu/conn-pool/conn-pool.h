#ifndef IDCU_CONN-POOL_CONN-POOL_H
#define IDCU_CONN-POOL_CONN-POOL_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/conn-pool/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_conn-pool_init(idcu_Conn-Pool_Context** ctx);
void idcu_conn-pool_destroy(idcu_Conn-Pool_Context* ctx);
int idcu_conn-pool_is_initialized(idcu_Conn-Pool_Context* ctx);
uint64_t idcu_conn-pool_get_operation_count(idcu_Conn-Pool_Context* ctx);
uint64_t idcu_conn-pool_get_error_count(idcu_Conn-Pool_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
