#ifndef IDCU_SANDBOX_SANDBOX_H
#define IDCU_SANDBOX_SANDBOX_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/sandbox/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_sandbox_init(idcu_Sandbox_Context** ctx);
void idcu_sandbox_destroy(idcu_Sandbox_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
