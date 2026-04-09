#ifndef IDCU_DISTRIBUTED_DISTRIBUTED_H
#define IDCU_DISTRIBUTED_DISTRIBUTED_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/distributed/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_distributed_init(idcu_Distributed_Context** ctx);
void idcu_distributed_destroy(idcu_Distributed_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
