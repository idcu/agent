#ifndef IDCU_CACHE_CACHE_H
#define IDCU_CACHE_CACHE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/cache/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_cache_init(idcu_Cache_Context** ctx);
void idcu_cache_destroy(idcu_Cache_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
