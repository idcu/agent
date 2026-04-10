#ifndef IDCU_CACHE_TYPES_H
#define IDCU_CACHE_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_CACHE_KEY_MAX        256
#define IDCU_CACHE_VALUE_MAX      65536
#define IDCU_CACHE_MAX_ENTRIES    100000
#define IDCU_CACHE_DEFAULT_TTL_MS 3600000

typedef enum {
    IDCU_CACHE_POLICY_LRU = 0,
    IDCU_CACHE_POLICY_LFU,
    IDCU_CACHE_POLICY_FIFO,
    IDCU_CACHE_POLICY_RANDOM
} idcu_CachePolicy;

typedef struct idcu_CacheEntry {
    char    key[IDCU_CACHE_KEY_MAX];
    uint8_t* value;
    size_t  value_size;
    uint64_t created_at;
    uint64_t last_accessed_at;
    uint64_t access_count;
    uint64_t expire_at;
    struct idcu_CacheEntry* prev;
    struct idcu_CacheEntry* next;
} idcu_CacheEntry;

typedef struct idcu_CacheConfig {
    idcu_CachePolicy policy;
    size_t max_entries;
    size_t max_memory;
    uint64_t default_ttl_ms;
} idcu_CacheConfig;

typedef struct idcu_Cache {
    idcu_CacheEntry** hash_table;
    uint32_t hash_table_size;
    idcu_CacheEntry* head;
    idcu_CacheEntry* tail;
    idcu_Mutex lock;
    idcu_CachePolicy policy;
    size_t max_entries;
    size_t current_entries;
    size_t max_memory;
    size_t current_memory;
    uint64_t default_ttl_ms;
    uint64_t hit_count;
    uint64_t miss_count;
    uint64_t evict_count;
    int initialized;
} idcu_Cache;

#ifdef __cplusplus
}
#endif

#endif
