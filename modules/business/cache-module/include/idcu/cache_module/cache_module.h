#ifndef IDCU_CACHE_MODULE_CACHE_MODULE_H
#define IDCU_CACHE_MODULE_CACHE_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/cache/cache.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_Cache* cache;
    int initialized;
    void* user_data;
} idcu_CacheModule;

int idcu_cache_module_init(idcu_CacheModule* cm);
int idcu_cache_module_start(idcu_CacheModule* cm);
int idcu_cache_module_stop(idcu_CacheModule* cm);
void idcu_cache_module_destroy(idcu_CacheModule* cm);

int idcu_cache_module_set(idcu_CacheModule* cm, const char* key, const void* value, size_t size, uint64_t ttl_ms);
int idcu_cache_module_get(idcu_CacheModule* cm, const char* key, void** value, size_t* size);
int idcu_cache_module_delete(idcu_CacheModule* cm, const char* key);

idcu_Cache* idcu_cache_module_get_cache(idcu_CacheModule* cm);

#ifdef __cplusplus
}
#endif

#endif
