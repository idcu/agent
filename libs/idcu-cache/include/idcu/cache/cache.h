#ifndef IDCU_CACHE_CACHE_H
#define IDCU_CACHE_CACHE_H

#include "idcu/common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_CACHE_KEY_MAX 256
#define IDCU_CACHE_VALUE_MAX 65536
#define IDCU_CACHE_MAX_ENTRIES 1024

typedef enum {
    IDCU_CACHE_POLICY_LRU = 0,
    IDCU_CACHE_POLICY_FIFO,
    IDCU_CACHE_POLICY_LFU
} idcu_CachePolicy;

typedef struct idcu_CacheEntry idcu_CacheEntry;
typedef struct idcu_Cache idcu_Cache;

int idcu_cache_init(idcu_Cache** cache, size_t max_entries, idcu_CachePolicy policy);
void idcu_cache_destroy(idcu_Cache* cache);

int idcu_cache_put(idcu_Cache* cache, const char* key, const void* value, size_t size, uint64_t ttl_ms);
int idcu_cache_get(idcu_Cache* cache, const char* key, void* buffer, size_t* buffer_size);
int idcu_cache_remove(idcu_Cache* cache, const char* key);
int idcu_cache_clear(idcu_Cache* cache);

int idcu_cache_contains(idcu_Cache* cache, const char* key);
size_t idcu_cache_size(idcu_Cache* cache);
size_t idcu_cache_count(idcu_Cache* cache);

int idcu_cache_put_string(idcu_Cache* cache, const char* key, const char* value, uint64_t ttl_ms);
int idcu_cache_get_string(idcu_Cache* cache, const char* key, char* buffer, size_t buffer_size);
int idcu_cache_put_int(idcu_Cache* cache, const char* key, int64_t value, uint64_t ttl_ms);
int idcu_cache_get_int(idcu_Cache* cache, const char* key, int64_t* value);

#endif // IDCU_CACHE_CACHE_H
