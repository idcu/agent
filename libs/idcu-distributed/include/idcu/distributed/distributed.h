#ifndef IDCU_DISTRIBUTED_DISTRIBUTED_H
#define IDCU_DISTRIBUTED_DISTRIBUTED_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/distributed/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_distributed_init(idcu_Distributed_Context** ctx);
void idcu_distributed_destroy(idcu_Distributed_Context* ctx);
int idcu_distributed_is_initialized(idcu_Distributed_Context* ctx);
uint64_t idcu_distributed_get_operation_count(idcu_Distributed_Context* ctx);
uint64_t idcu_distributed_get_error_count(idcu_Distributed_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
