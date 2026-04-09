#ifndef IDCU_MEMORY_MEMORY_H
#define IDCU_MEMORY_MEMORY_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/memory/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_memory_init(idcu_Memory_Context** ctx);
void idcu_memory_destroy(idcu_Memory_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
