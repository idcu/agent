#include "cache.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

struct idcu_CacheEntry {
    char key[IDCU_CACHE_KEY_MAX];
    void* value;
    size_t value_size;
    uint64_t expire_time;
    uint64_t access_time;
    uint64_t insert_time;
    uint32_t access_count;
    struct idcu_CacheEntry* prev;
    struct idcu_CacheEntry* next;
};

struct idcu_Cache {
    idcu_CacheEntry* entries;
    idcu_CacheEntry* head;
    idcu_CacheEntry* tail;
    idcu_CachePolicy policy;
    size_t max_entries;
    size_t current_count;
    size_t total_size;
    uint64_t hits;
    uint64_t misses;
};

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

static void remove_entry_from_list(idcu_Cache* cache, idcu_CacheEntry* entry) {
    if (!entry || !cache) return;

    if (entry->prev) {
        entry->prev->next = entry->next;
    } else {
        cache->head = entry->next;
    }

    if (entry->next) {
        entry->next->prev = entry->prev;
    } else {
        cache->tail = entry->prev;
    }
}

static void add_entry_to_head(idcu_Cache* cache, idcu_CacheEntry* entry) {
    if (!entry || !cache) return;

    entry->prev = NULL;
    entry->next = cache->head;

    if (cache->head) {
        cache->head->prev = entry;
    }
    cache->head = entry;

    if (!cache->tail) {
        cache->tail = entry;
    }
}

static void evict_entry(idcu_Cache* cache) {
    if (!cache || !cache->tail) return;

    idcu_CacheEntry* to_remove = cache->tail;
    remove_entry_from_list(cache, to_remove);

    if (to_remove->value) {
        free(to_remove->value);
        to_remove->value = NULL;
    }

    cache->total_size -= to_remove->value_size;
    memset(to_remove, 0, sizeof(idcu_CacheEntry));
    cache->current_count--;
}

static idcu_CacheEntry* find_entry(idcu_Cache* cache, const char* key) {
    if (!cache || !key) return NULL;

    uint64_t now = get_current_time_ms();
    idcu_CacheEntry* entry = cache->head;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (entry->expire_time > 0 && entry->expire_time < now) {
                remove_entry_from_list(cache, entry);
                if (entry->value) {
                    free(entry->value);
                    entry->value = NULL;
                }
                cache->total_size -= entry->value_size;
                memset(entry, 0, sizeof(idcu_CacheEntry));
                cache->current_count--;
                return NULL;
            }
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

int idcu_cache_init(idcu_Cache** cache, size_t max_entries, idcu_CachePolicy policy) {
    if (!cache) return IDCU_ERR_INVALID_PARAM;

    if (max_entries == 0) {
        max_entries = IDCU_CACHE_MAX_ENTRIES;
    }

    *cache = (idcu_Cache*)malloc(sizeof(idcu_Cache));
    if (!*cache) {
        return IDCU_ERR_NO_MEMORY;
    }

    memset(*cache, 0, sizeof(idcu_Cache));

    (*cache)->entries = (idcu_CacheEntry*)calloc(max_entries, sizeof(idcu_CacheEntry));
    if (!(*cache)->entries) {
        free(*cache);
        *cache = NULL;
        return IDCU_ERR_NO_MEMORY;
    }

    (*cache)->policy = policy;
    (*cache)->max_entries = max_entries;
    (*cache)->head = NULL;
    (*cache)->tail = NULL;
    (*cache)->current_count = 0;
    (*cache)->total_size = 0;
    (*cache)->hits = 0;
    (*cache)->misses = 0;

    IDCU_LOG_INFO("[cache] Initialized with policy %d, max entries %zu", policy, max_entries);
    return IDCU_ERR_OK;
}

void idcu_cache_destroy(idcu_Cache* cache) {
    if (!cache) return;

    idcu_cache_clear(cache);

    if (cache->entries) {
        free(cache->entries);
        cache->entries = NULL;
    }

    free(cache);
    IDCU_LOG_INFO("[cache] Destroyed");
}

int idcu_cache_put(idcu_Cache* cache, const char* key, const void* value, size_t size, uint64_t ttl_ms) {
    if (!cache || !key || !value || size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (size > IDCU_CACHE_VALUE_MAX) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint64_t now = get_current_time_ms();

    idcu_CacheEntry* existing = find_entry(cache, key);
    if (existing) {
        remove_entry_from_list(cache, existing);
        if (existing->value) {
            free(existing->value);
            existing->value = NULL;
        }
        cache->total_size -= existing->value_size;
    } else if (cache->current_count >= cache->max_entries) {
        evict_entry(cache);
    }

    idcu_CacheEntry* entry = NULL;
    for (size_t i = 0; i < cache->max_entries; i++) {
        if (cache->entries[i].key[0] == '\0') {
            entry = &cache->entries[i];
            break;
        }
    }

    if (!entry) {
        return IDCU_ERR_NO_MEMORY;
    }

    strncpy(entry->key, key, IDCU_CACHE_KEY_MAX - 1);
    entry->key[IDCU_CACHE_KEY_MAX - 1] = '\0';

    entry->value = malloc(size);
    if (!entry->value) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(entry->value, value, size);
    entry->value_size = size;

    entry->insert_time = now;
    entry->access_time = now;
    entry->access_count = 1;
    entry->expire_time = ttl_ms > 0 ? (now + ttl_ms) : 0;

    add_entry_to_head(cache, entry);
    cache->current_count++;
    cache->total_size += size;

    IDCU_LOG_DEBUG("[cache] Put key: %s, size: %zu, ttl: %llu", key, size, (unsigned long long)ttl_ms);
    return IDCU_ERR_OK;
}

int idcu_cache_get(idcu_Cache* cache, const char* key, void* buffer, size_t* buffer_size) {
    if (!cache || !key || !buffer || !buffer_size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_CacheEntry* entry = find_entry(cache, key);
    if (!entry) {
        cache->misses++;
        return IDCU_ERR_NOT_FOUND;
    }

    if (*buffer_size < entry->value_size) {
        *buffer_size = entry->value_size;
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }

    memcpy(buffer, entry->value, entry->value_size);
    *buffer_size = entry->value_size;

    entry->access_time = get_current_time_ms();
    entry->access_count++;

    if (cache->policy == IDCU_CACHE_POLICY_LRU) {
        remove_entry_from_list(cache, entry);
        add_entry_to_head(cache, entry);
    }

    cache->hits++;
    IDCU_LOG_DEBUG("[cache] Hit key: %s", key);
    return IDCU_ERR_OK;
}

int idcu_cache_remove(idcu_Cache* cache, const char* key) {
    if (!cache || !key) return IDCU_ERR_INVALID_PARAM;

    idcu_CacheEntry* entry = find_entry(cache, key);
    if (!entry) {
        return IDCU_ERR_NOT_FOUND;
    }

    remove_entry_from_list(cache, entry);
    if (entry->value) {
        free(entry->value);
        entry->value = NULL;
    }
    cache->total_size -= entry->value_size;
    memset(entry, 0, sizeof(idcu_CacheEntry));
    cache->current_count--;

    IDCU_LOG_DEBUG("[cache] Removed key: %s", key);
    return IDCU_ERR_OK;
}

int idcu_cache_clear(idcu_Cache* cache) {
    if (!cache) return IDCU_ERR_INVALID_PARAM;

    idcu_CacheEntry* entry = cache->head;
    while (entry) {
        idcu_CacheEntry* next = entry->next;
        if (entry->value) {
            free(entry->value);
            entry->value = NULL;
        }
        memset(entry, 0, sizeof(idcu_CacheEntry));
        entry = next;
    }

    cache->head = NULL;
    cache->tail = NULL;
    cache->current_count = 0;
    cache->total_size = 0;

    IDCU_LOG_INFO("[cache] Cleared");
    return IDCU_ERR_OK;
}

int idcu_cache_contains(idcu_Cache* cache, const char* key) {
    if (!cache || !key) return 0;
    return find_entry(cache, key) != NULL;
}

size_t idcu_cache_size(idcu_Cache* cache) {
    if (!cache) return 0;
    return cache->total_size;
}

size_t idcu_cache_count(idcu_Cache* cache) {
    if (!cache) return 0;
    return cache->current_count;
}

int idcu_cache_put_string(idcu_Cache* cache, const char* key, const char* value, uint64_t ttl_ms) {
    if (!cache || !key || !value) return IDCU_ERR_INVALID_PARAM;
    return idcu_cache_put(cache, key, value, strlen(value) + 1, ttl_ms);
}

int idcu_cache_get_string(idcu_Cache* cache, const char* key, char* buffer, size_t buffer_size) {
    if (!cache || !key || !buffer || buffer_size == 0) return IDCU_ERR_INVALID_PARAM;
    return idcu_cache_get(cache, key, buffer, &buffer_size);
}

int idcu_cache_put_int(idcu_Cache* cache, const char* key, int64_t value, uint64_t ttl_ms) {
    if (!cache || !key) return IDCU_ERR_INVALID_PARAM;
    return idcu_cache_put(cache, key, &value, sizeof(int64_t), ttl_ms);
}

int idcu_cache_get_int(idcu_Cache* cache, const char* key, int64_t* value) {
    if (!cache || !key || !value) return IDCU_ERR_INVALID_PARAM;
    size_t size = sizeof(int64_t);
    return idcu_cache_get(cache, key, value, &size);
}
