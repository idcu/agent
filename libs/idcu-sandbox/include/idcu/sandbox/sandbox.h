#ifndef IDCU_SANDBOX_SANDBOX_H
#define IDCU_SANDBOX_SANDBOX_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/sandbox/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_sandbox_init(idcu_Sandbox_Context** ctx);
void idcu_sandbox_destroy(idcu_Sandbox_Context* ctx);
int idcu_sandbox_is_initialized(idcu_Sandbox_Context* ctx);
uint64_t idcu_sandbox_get_operation_count(idcu_Sandbox_Context* ctx);
uint64_t idcu_sandbox_get_error_count(idcu_Sandbox_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
