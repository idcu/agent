#include <idcu/cache/cache.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define HASH_TABLE_SIZE 1024

static uint64_t get_timestamp_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (uli.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

static uint32_t hash_key(const char* key) {
    uint32_t hash = 5381;
    while (*key) {
        hash = ((hash << 5) + hash) + (unsigned char)*key++;
    }
    return hash;
}

static idcu_CacheEntry* find_entry(idcu_Cache* cache, const char* key) {
    uint32_t idx = hash_key(key) % cache->hash_table_size;
    idcu_CacheEntry* entry = cache->hash_table[idx];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static void remove_from_list(idcu_Cache* cache, idcu_CacheEntry* entry) {
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
    
    entry->prev = NULL;
    entry->next = NULL;
}

static void add_to_head(idcu_Cache* cache, idcu_CacheEntry* entry) {
    entry->next = cache->head;
    entry->prev = NULL;
    
    if (cache->head) {
        cache->head->prev = entry;
    }
    cache->head = entry;
    
    if (!cache->tail) {
        cache->tail = entry;
    }
}

static void add_to_tail(idcu_Cache* cache, idcu_CacheEntry* entry) {
    entry->prev = cache->tail;
    entry->next = NULL;
    
    if (cache->tail) {
        cache->tail->next = entry;
    }
    cache->tail = entry;
    
    if (!cache->head) {
        cache->head = entry;
    }
}

static int is_entry_expired(idcu_Cache* cache, idcu_CacheEntry* entry) {
    if (entry->expire_at == 0) {
        return 0;
    }
    return get_timestamp_ms() > entry->expire_at;
}

static void evict_entry(idcu_Cache* cache, idcu_CacheEntry* entry) {
    uint32_t idx = hash_key(entry->key) % cache->hash_table_size;
    idcu_CacheEntry** head_ptr = &cache->hash_table[idx];
    
    if (*head_ptr == entry) {
        *head_ptr = entry->next;
    } else {
        idcu_CacheEntry* prev = *head_ptr;
        while (prev && prev->next != entry) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = entry->next;
        }
    }
    
    remove_from_list(cache, entry);
    
    cache->current_entries--;
    cache->current_memory -= entry->value_size;
    
    if (entry->value) {
        free(entry->value);
    }
    free(entry);
    
    cache->evict_count++;
}

static idcu_CacheEntry* find_eviction_candidate(idcu_Cache* cache) {
    idcu_CacheEntry* candidate = NULL;
    
    switch (cache->policy) {
        case IDCU_CACHE_POLICY_LRU:
            candidate = cache->tail;
            break;
            
        case IDCU_CACHE_POLICY_FIFO:
            candidate = cache->head;
            break;
            
        case IDCU_CACHE_POLICY_LFU: {
            uint64_t min_count = (uint64_t)-1;
            idcu_CacheEntry* entry = cache->head;
            while (entry) {
                if (entry->access_count < min_count) {
                    min_count = entry->access_count;
                    candidate = entry;
                }
                entry = entry->next;
            }
            break;
        }
        
        case IDCU_CACHE_POLICY_RANDOM: {
            uint32_t idx = (uint32_t)rand() % cache->current_entries;
            idcu_CacheEntry* entry = cache->head;
            for (uint32_t i = 0; i < idx && entry; i++) {
                entry = entry->next;
            }
            candidate = entry;
            break;
        }
    }
    
    return candidate;
}

static void cleanup_expired_entries(idcu_Cache* cache) {
    idcu_CacheEntry* entry = cache->head;
    while (entry) {
        idcu_CacheEntry* next = entry->next;
        if (is_entry_expired(cache, entry)) {
            evict_entry(cache, entry);
        }
        entry = next;
    }
}

static int ensure_capacity(idcu_Cache* cache, size_t required_memory) {
    while (cache->current_entries >= cache->max_entries ||
           (cache->max_memory > 0 && cache->current_memory + required_memory > cache->max_memory)) {
        idcu_CacheEntry* candidate = find_eviction_candidate(cache);
        if (!candidate) {
            return IDCU_ERR_OUT_OF_RANGE;
        }
        evict_entry(cache, candidate);
    }
    return IDCU_ERR_OK;
}

int idcu_cache_init(idcu_Cache* cache, const idcu_CacheConfig* config) {
    if (!cache || !config) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(cache, 0, sizeof(*cache));
    
    cache->hash_table_size = HASH_TABLE_SIZE;
    cache->hash_table = (idcu_CacheEntry**)calloc(cache->hash_table_size, sizeof(idcu_CacheEntry*));
    if (!cache->hash_table) {
        return IDCU_ERR_MEMORY;
    }
    
    idcu_mutex_init(&cache->lock);
    
    cache->policy = config->policy;
    cache->max_entries = config->max_entries > 0 ? config->max_entries : 1000;
    cache->max_memory = config->max_memory;
    cache->default_ttl_ms = config->default_ttl_ms > 0 ? config->default_ttl_ms : IDCU_CACHE_DEFAULT_TTL_MS;
    
    cache->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_cache_destroy(idcu_Cache* cache) {
    if (!cache || !cache->initialized) {
        return;
    }
    
    idcu_mutex_lock(&cache->lock);
    
    idcu_CacheEntry* entry = cache->head;
    while (entry) {
        idcu_CacheEntry* next = entry->next;
        if (entry->value) {
            free(entry->value);
        }
        free(entry);
        entry = next;
    }
    
    if (cache->hash_table) {
        free(cache->hash_table);
    }
    
    idcu_Mutex lock_copy = cache->lock;
    cache->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_cache_put_ttl(idcu_Cache* cache, const char* key, const void* value, size_t value_size, uint64_t ttl_ms) {
    if (!cache || !cache->initialized || !key || !value || value_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (strlen(key) >= IDCU_CACHE_KEY_MAX || value_size > IDCU_CACHE_VALUE_MAX) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    idcu_mutex_lock(&cache->lock);
    
    cleanup_expired_entries(cache);
    
    idcu_CacheEntry* existing = find_entry(cache, key);
    if (existing) {
        uint8_t* new_value = (uint8_t*)realloc(existing->value, value_size);
        if (!new_value) {
            idcu_mutex_unlock(&cache->lock);
            return IDCU_ERR_MEMORY;
        }
        
        cache->current_memory -= existing->value_size;
        existing->value = new_value;
        memcpy(existing->value, value, value_size);
        existing->value_size = value_size;
        cache->current_memory += value_size;
        
        existing->last_accessed_at = get_timestamp_ms();
        existing->access_count++;
        
        if (ttl_ms > 0) {
            existing->expire_at = get_timestamp_ms() + ttl_ms;
        }
        
        if (cache->policy == IDCU_CACHE_POLICY_LRU) {
            remove_from_list(cache, existing);
            add_to_head(cache, existing);
        }
        
        idcu_mutex_unlock(&cache->lock);
        return IDCU_ERR_OK;
    }
    
    if (ensure_capacity(cache, value_size) != IDCU_ERR_OK) {
        idcu_mutex_unlock(&cache->lock);
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    idcu_CacheEntry* entry = (idcu_CacheEntry*)calloc(1, sizeof(idcu_CacheEntry));
    if (!entry) {
        idcu_mutex_unlock(&cache->lock);
        return IDCU_ERR_MEMORY;
    }
    
    strncpy(entry->key, key, IDCU_CACHE_KEY_MAX - 1);
    entry->key[IDCU_CACHE_KEY_MAX - 1] = '\0';
    
    entry->value = (uint8_t*)malloc(value_size);
    if (!entry->value) {
        free(entry);
        idcu_mutex_unlock(&cache->lock);
        return IDCU_ERR_MEMORY;
    }
    
    memcpy(entry->value, value, value_size);
    entry->value_size = value_size;
    
    uint64_t now = get_timestamp_ms();
    entry->created_at = now;
    entry->last_accessed_at = now;
    entry->access_count = 1;
    
    if (ttl_ms > 0) {
        entry->expire_at = now + ttl_ms;
    } else {
        entry->expire_at = now + cache->default_ttl_ms;
    }
    
    uint32_t idx = hash_key(key) % cache->hash_table_size;
    entry->next = cache->hash_table[idx];
    cache->hash_table[idx] = entry;
    
    if (cache->policy == IDCU_CACHE_POLICY_FIFO) {
        add_to_tail(cache, entry);
    } else {
        add_to_head(cache, entry);
    }
    
    cache->current_entries++;
    cache->current_memory += value_size;
    
    idcu_mutex_unlock(&cache->lock);
    return IDCU_ERR_OK;
}

int idcu_cache_put(idcu_Cache* cache, const char* key, const void* value, size_t value_size) {
    return idcu_cache_put_ttl(cache, key, value, value_size, 0);
}

int idcu_cache_put_string(idcu_Cache* cache, const char* key, const char* value) {
    if (!value) {
        return IDCU_ERR_INVALID_ARG;
    }
    return idcu_cache_put(cache, key, value, strlen(value) + 1);
}

int idcu_cache_put_int(idcu_Cache* cache, const char* key, int64_t value) {
    return idcu_cache_put(cache, key, &value, sizeof(value));
}

int idcu_cache_get(idcu_Cache* cache, const char* key, void* buffer, size_t buffer_size, size_t* value_size) {
    if (!cache || !cache->initialized || !key || !buffer) {
        if (cache) cache->miss_count++;
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&cache->lock);
    
    idcu_CacheEntry* entry = find_entry(cache, key);
    if (!entry || is_entry_expired(cache, entry)) {
        if (entry) {
            evict_entry(cache, entry);
        }
        cache->miss_count++;
        idcu_mutex_unlock(&cache->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    
    cache->hit_count++;
    entry->access_count++;
    entry->last_accessed_at = get_timestamp_ms();
    
    if (value_size) {
        *value_size = entry->value_size;
    }
    
    if (buffer_size < entry->value_size) {
        idcu_mutex_unlock(&cache->lock);
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    memcpy(buffer, entry->value, entry->value_size);
    
    if (cache->policy == IDCU_CACHE_POLICY_LRU) {
        remove_from_list(cache, entry);
        add_to_head(cache, entry);
    }
    
    idcu_mutex_unlock(&cache->lock);
    return IDCU_ERR_OK;
}

int idcu_cache_get_string(idcu_Cache* cache, const char* key, char* buffer, size_t buffer_size) {
    size_t value_size;
    int ret = idcu_cache_get(cache, key, buffer, buffer_size, &value_size);
    if (ret == IDCU_ERR_OK && buffer_size > 0) {
        buffer[buffer_size - 1] = '\0';
    }
    return ret;
}

int idcu_cache_get_int(idcu_Cache* cache, const char* key, int64_t* out_value) {
    if (!out_value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t value_size;
    int ret = idcu_cache_get(cache, key, out_value, sizeof(*out_value), &value_size);
    if (ret == IDCU_ERR_OK && value_size != sizeof(*out_value)) {
        return IDCU_ERR_INVALID_ARG;
    }
    return ret;
}

int idcu_cache_remove(idcu_Cache* cache, const char* key) {
    if (!cache || !cache->initialized || !key) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&cache->lock);
    
    idcu_CacheEntry* entry = find_entry(cache, key);
    if (!entry) {
        idcu_mutex_unlock(&cache->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    
    evict_entry(cache, entry);
    
    idcu_mutex_unlock(&cache->lock);
    return IDCU_ERR_OK;
}

int idcu_cache_clear(idcu_Cache* cache) {
    if (!cache || !cache->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&cache->lock);
    
    idcu_CacheEntry* entry = cache->head;
    while (entry) {
        idcu_CacheEntry* next = entry->next;
        if (entry->value) {
            free(entry->value);
        }
        free(entry);
        entry = next;
    }
    
    memset(cache->hash_table, 0, cache->hash_table_size * sizeof(idcu_CacheEntry*));
    cache->head = NULL;
    cache->tail = NULL;
    cache->current_entries = 0;
    cache->current_memory = 0;
    
    idcu_mutex_unlock(&cache->lock);
    return IDCU_ERR_OK;
}

int idcu_cache_contains(idcu_Cache* cache, const char* key) {
    if (!cache || !cache->initialized || !key) {
        return 0;
    }
    
    idcu_mutex_lock(&cache->lock);
    
    idcu_CacheEntry* entry = find_entry(cache, key);
    int result = (entry != NULL && !is_entry_expired(cache, entry));
    
    idcu_mutex_unlock(&cache->lock);
    return result;
}

size_t idcu_cache_get_count(idcu_Cache* cache) {
    if (!cache || !cache->initialized) {
        return 0;
    }
    return cache->current_entries;
}

size_t idcu_cache_get_memory_usage(idcu_Cache* cache) {
    if (!cache || !cache->initialized) {
        return 0;
    }
    return cache->current_memory;
}

uint64_t idcu_cache_get_hit_count(idcu_Cache* cache) {
    if (!cache) return 0;
    return cache->hit_count;
}

uint64_t idcu_cache_get_miss_count(idcu_Cache* cache) {
    if (!cache) return 0;
    return cache->miss_count;
}

uint64_t idcu_cache_get_evict_count(idcu_Cache* cache) {
    if (!cache) return 0;
    return cache->evict_count;
}

double idcu_cache_get_hit_rate(idcu_Cache* cache) {
    if (!cache) return 0.0;
    uint64_t total = cache->hit_count + cache->miss_count;
    if (total == 0) return 0.0;
    return (double)cache->hit_count / (double)total;
}
