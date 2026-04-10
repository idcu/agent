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

int  idcu_cache_init(idcu_Cache* cache, const idcu_CacheConfig* config);
void idcu_cache_destroy(idcu_Cache* cache);

int  idcu_cache_put(idcu_Cache* cache, const char* key, const void* value, size_t value_size);
int  idcu_cache_put_ttl(idcu_Cache* cache, const char* key, const void* value, size_t value_size, uint64_t ttl_ms);
int  idcu_cache_put_string(idcu_Cache* cache, const char* key, const char* value);
int  idcu_cache_put_int(idcu_Cache* cache, const char* key, int64_t value);

int  idcu_cache_get(idcu_Cache* cache, const char* key, void* buffer, size_t buffer_size, size_t* value_size);
int  idcu_cache_get_string(idcu_Cache* cache, const char* key, char* buffer, size_t buffer_size);
int  idcu_cache_get_int(idcu_Cache* cache, const char* key, int64_t* out_value);

int  idcu_cache_remove(idcu_Cache* cache, const char* key);
int  idcu_cache_clear(idcu_Cache* cache);
int  idcu_cache_contains(idcu_Cache* cache, const char* key);

size_t idcu_cache_get_count(idcu_Cache* cache);
size_t idcu_cache_get_memory_usage(idcu_Cache* cache);

uint64_t idcu_cache_get_hit_count(idcu_Cache* cache);
uint64_t idcu_cache_get_miss_count(idcu_Cache* cache);
uint64_t idcu_cache_get_evict_count(idcu_Cache* cache);
double idcu_cache_get_hit_rate(idcu_Cache* cache);

#ifdef __cplusplus
}
#endif

#endif
