#ifndef IDCU_CACHE_CACHE_H
#define IDCU_CACHE_CACHE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/cache/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_cache_init(idcu_Cache_Context** ctx);
void idcu_cache_destroy(idcu_Cache_Context* ctx);
int idcu_cache_is_initialized(idcu_Cache_Context* ctx);
uint64_t idcu_cache_get_operation_count(idcu_Cache_Context* ctx);
uint64_t idcu_cache_get_error_count(idcu_Cache_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
