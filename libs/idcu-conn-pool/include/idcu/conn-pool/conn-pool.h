#ifndef IDCU_CONN-POOL_CONN-POOL_H
#define IDCU_CONN-POOL_CONN-POOL_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/conn-pool/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_conn-pool_init(idcu_Conn-Pool_Context** ctx);
void idcu_conn-pool_destroy(idcu_Conn-Pool_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
