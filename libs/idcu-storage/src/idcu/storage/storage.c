#include <idcu/storage/storage.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 32

// Cache statistics
static size_t g_cache_hits = 0;
static size_t g_cache_misses = 0;

// ========== Cache internal functions ==========
static uint64_t get_timestamp_ms(void) {
    // Simple timestamp implementation
    static uint64_t counter = 0;
    return ++counter;
}

static void cache_add_to_head(idcu_KVStore* store, idcu_KVCacheEntry* entry) {
    entry->prev = NULL;
    entry->next = store->cache_head;
    
    if (store->cache_head) {
        store->cache_head->prev = entry;
    }
    store->cache_head = entry;
    
    if (!store->cache_tail) {
        store->cache_tail = entry;
    }
}

static void cache_remove_entry(idcu_KVStore* store, idcu_KVCacheEntry* entry) {
    if (entry->prev) {
        entry->prev->next = entry->next;
    } else {
        store->cache_head = entry->next;
    }
    
    if (entry->next) {
        entry->next->prev = entry->prev;
    } else {
        store->cache_tail = entry->prev;
    }
    
    store->cache_entry_count--;
    store->cache_memory_usage -= entry->value_size;
    
    if (entry->value) {
        free(entry->value);
    }
    free(entry);
}

static void cache_evict_lru(idcu_KVStore* store) {
    if (store->cache_tail) {
        cache_remove_entry(store, store->cache_tail);
    }
}

static idcu_KVCacheEntry* cache_find(idcu_KVStore* store, const char* key) {
    idcu_KVCacheEntry* entry = store->cache_head;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Move to head (MRU)
            if (entry != store->cache_head) {
                cache_remove_entry(store, entry);
                cache_add_to_head(store, entry);
            }
            entry->last_access = get_timestamp_ms();
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static void cache_put(idcu_KVStore* store, const char* key, const void* value, size_t value_size) {
    if (!store->cache_config.enabled) {
        return;
    }
    
    // Check if already exists
    idcu_KVCacheEntry* existing = cache_find(store, key);
    if (existing) {
        // Update existing
        if (existing->value) {
            store->cache_memory_usage -= existing->value_size;
            free(existing->value);
        }
        
        existing->value = (uint8_t*)malloc(value_size);
        if (existing->value) {
            memcpy(existing->value, value, value_size);
            existing->value_size = value_size;
            store->cache_memory_usage += value_size;
            existing->last_access = get_timestamp_ms();
        }
        return;
    }
    
    // Evict if needed
    while (store->cache_entry_count >= store->cache_config.max_entries ||
           (store->cache_config.max_memory_bytes > 0 && 
            store->cache_memory_usage + value_size > store->cache_config.max_memory_bytes)) {
        cache_evict_lru(store);
    }
    
    // Create new entry
    idcu_KVCacheEntry* new_entry = (idcu_KVCacheEntry*)malloc(sizeof(idcu_KVCacheEntry));
    if (!new_entry) {
        return;
    }
    
    strncpy(new_entry->key, key, IDCU_STORAGE_KEY_MAX - 1);
    new_entry->key[IDCU_STORAGE_KEY_MAX - 1] = '\0';
    
    new_entry->value = (uint8_t*)malloc(value_size);
    if (!new_entry->value) {
        free(new_entry);
        return;
    }
    
    memcpy(new_entry->value, value, value_size);
    new_entry->value_size = value_size;
    new_entry->last_access = get_timestamp_ms();
    
    cache_add_to_head(store, new_entry);
    store->cache_entry_count++;
    store->cache_memory_usage += value_size;
}

// ========== Compression (simple RLE for demonstration) ==========
static size_t simple_compress(const uint8_t* src, size_t src_len, uint8_t* dst, size_t dst_len) {
    // Simple run-length encoding for demonstration
    // In production, use zlib or similar
    size_t src_idx = 0;
    size_t dst_idx = 0;
    
    while (src_idx < src_len && dst_idx + 2 < dst_len) {
        uint8_t current = src[src_idx];
        size_t run_length = 1;
        
        while (src_idx + run_length < src_len && 
               src[src_idx + run_length] == current && 
               run_length < 255) {
            run_length++;
        }
        
        dst[dst_idx++] = current;
        dst[dst_idx++] = (uint8_t)run_length;
        src_idx += run_length;
    }
    
    return dst_idx;
}

static size_t simple_decompress(const uint8_t* src, size_t src_len, uint8_t* dst, size_t dst_len) {
    size_t src_idx = 0;
    size_t dst_idx = 0;
    
    while (src_idx + 1 < src_len && dst_idx < dst_len) {
        uint8_t value = src[src_idx++];
        uint8_t count = src[src_idx++];
        
        for (uint8_t i = 0; i < count && dst_idx < dst_len; i++) {
            dst[dst_idx++] = value;
        }
    }
    
    return dst_idx;
}

// ========== 内部辅助函数 ==========
static int find_entry_index(idcu_KVStore* store, const char* key) {
    for (uint32_t i = 0; i < store->entry_count; i++) {
        if (strcmp(store->entries[i].key, key) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int ensure_capacity(idcu_KVStore* store) {
    if (store->entry_count < store->capacity) {
        return IDCU_ERR_OK;
    }
    
    uint32_t new_capacity = store->capacity == 0 ? INITIAL_CAPACITY : store->capacity * 2;
    if (new_capacity > IDCU_STORAGE_MAX_ENTRIES) {
        new_capacity = IDCU_STORAGE_MAX_ENTRIES;
    }
    
    if (new_capacity <= store->capacity) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    idcu_KVEntry* new_entries = (idcu_KVEntry*)realloc(store->entries, new_capacity * sizeof(idcu_KVEntry));
    if (!new_entries) {
        return IDCU_ERR_MEMORY;
    }
    
    store->entries = new_entries;
    store->capacity = new_capacity;
    return IDCU_ERR_OK;
}

static int save_to_json(idcu_KVStore* store) {
    FILE* fp = fopen(store->path, "w");
    if (!fp) {
        return IDCU_ERR_UNKNOWN;
    }
    
    fprintf(fp, "{\n");
    for (uint32_t i = 0; i < store->entry_count; i++) {
        fprintf(fp, "  \"%s\": {", store->entries[i].key);
        fprintf(fp, "\"size\": %zu,", store->entries[i].value_size);
        
        // 以十六进制保存二进制数据
        fprintf(fp, "\"data\": \"");
        for (size_t j = 0; j < store->entries[i].value_size; j++) {
            fprintf(fp, "%02x", store->entries[i].value[j]);
        }
        fprintf(fp, "\"}");
        
        if (i < store->entry_count - 1) {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "}\n");
    
    fclose(fp);
    return IDCU_ERR_OK;
}

static int load_from_json(idcu_KVStore* store) {
    FILE* fp = fopen(store->path, "r");
    if (!fp) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    // 清空现有数据
    for (uint32_t i = 0; i < store->entry_count; i++) {
        if (store->entries[i].value) {
            free(store->entries[i].value);
        }
    }
    store->entry_count = 0;
    
    // 简单解析JSON格式
    char line[8192];
    while (fgets(line, sizeof(line), fp)) {
        char* key_start = strchr(line, '"');
        if (!key_start) continue;
        key_start++;
        
        char* key_end = strchr(key_start, '"');
        if (!key_end) continue;
        
        size_t key_len = key_end - key_start;
        if (key_len >= IDCU_STORAGE_KEY_MAX) continue;
        
        // 查找size
        char* size_str = strstr(key_end, "\"size\":");
        if (!size_str) continue;
        
        size_t size = 0;
        if (sscanf(size_str, "\"size\": %zu,", &size) != 1) continue;
        
        // 查找data
        char* data_str = strstr(size_str, "\"data\": \"");
        if (!data_str) continue;
        data_str += 9;
        
        char* data_end = strchr(data_str, '"');
        if (!data_end) continue;
        
        size_t hex_len = data_end - data_str;
        if (hex_len != size * 2) continue;
        
        // 解析十六进制数据
        uint8_t* value = (uint8_t*)malloc(size);
        if (!value) {
            fclose(fp);
            return IDCU_ERR_MEMORY;
        }
        
        for (size_t i = 0; i < size; i++) {
            unsigned int byte;
            sscanf(data_str + i * 2, "%02x", &byte);
            value[i] = (uint8_t)byte;
        }
        
        // 添加到存储
        if (ensure_capacity(store) != IDCU_ERR_OK) {
            free(value);
            fclose(fp);
            return IDCU_ERR_OUT_OF_RANGE;
        }
        
        strncpy(store->entries[store->entry_count].key, key_start, key_len);
        store->entries[store->entry_count].key[key_len] = '\0';
        store->entries[store->entry_count].value = value;
        store->entries[store->entry_count].value_size = size;
        store->entry_count++;
    }
    
    fclose(fp);
    return IDCU_ERR_OK;
}

// ========== KV存储API ==========
int idcu_kvstore_init(idcu_KVStore* store, const char* path) {
    if (!store || !path) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(store, 0, sizeof(*store));
    strncpy(store->path, path, IDCU_STORAGE_PATH_MAX - 1);
    store->path[IDCU_STORAGE_PATH_MAX - 1] = '\0';
    
    idcu_mutex_init(&store->lock);
    store->capacity = 0;
    store->entry_count = 0;
    store->entries = NULL;
    store->dirty = 0;
    store->initialized = 1;
    
    // 尝试加载现有数据
    load_from_json(store);
    return IDCU_ERR_OK;
}

void idcu_kvstore_destroy(idcu_KVStore* store) {
    if (!store || !store->initialized) {
        return;
    }
    
    idcu_mutex_lock(&store->lock);
    
    // 同步到磁盘
    if (store->dirty) {
        save_to_json(store);
    }
    
    // 释放内存
    for (uint32_t i = 0; i < store->entry_count; i++) {
        if (store->entries[i].value) {
            free(store->entries[i].value);
        }
    }
    
    if (store->entries) {
        free(store->entries);
    }
    
    idcu_Mutex lock_copy = store->lock;
    store->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_kvstore_put(idcu_KVStore* store, const char* key, const void* value, size_t value_size) {
    if (!store || !store->initialized || !key || !value || value_size == 0) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (value_size > IDCU_STORAGE_VALUE_MAX) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    idcu_mutex_lock(&store->lock);
    
    int idx = find_entry_index(store, key);
    if (idx >= 0) {
        // 更新现有条目
        uint8_t* new_value = (uint8_t*)realloc(store->entries[idx].value, value_size);
        if (!new_value) {
            idcu_mutex_unlock(&store->lock);
            return IDCU_ERR_MEMORY;
        }
        store->entries[idx].value = new_value;
        memcpy(store->entries[idx].value, value, value_size);
        store->entries[idx].value_size = value_size;
    } else {
        // 添加新条目
        if (ensure_capacity(store) != IDCU_ERR_OK) {
            idcu_mutex_unlock(&store->lock);
            return IDCU_ERR_OUT_OF_RANGE;
        }
        
        strncpy(store->entries[store->entry_count].key, key, IDCU_STORAGE_KEY_MAX - 1);
        store->entries[store->entry_count].key[IDCU_STORAGE_KEY_MAX - 1] = '\0';
        
        store->entries[store->entry_count].value = (uint8_t*)malloc(value_size);
        if (!store->entries[store->entry_count].value) {
            idcu_mutex_unlock(&store->lock);
            return IDCU_ERR_MEMORY;
        }
        
        memcpy(store->entries[store->entry_count].value, value, value_size);
        store->entries[store->entry_count].value_size = value_size;
        store->entry_count++;
    }
    
    store->dirty = 1;
    idcu_mutex_unlock(&store->lock);
    return IDCU_ERR_OK;
}

int idcu_kvstore_put_string(idcu_KVStore* store, const char* key, const char* value) {
    if (!value) {
        return IDCU_ERR_INVALID_ARG;
    }
    return idcu_kvstore_put(store, key, value, strlen(value) + 1);
}

int idcu_kvstore_put_int(idcu_KVStore* store, const char* key, int64_t value) {
    return idcu_kvstore_put(store, key, &value, sizeof(value));
}

int idcu_kvstore_put_double(idcu_KVStore* store, const char* key, double value) {
    return idcu_kvstore_put(store, key, &value, sizeof(value));
}

int idcu_kvstore_get(idcu_KVStore* store, const char* key, void* buffer, size_t buffer_size, size_t* value_size) {
    if (!store || !store->initialized || !key || !buffer) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&store->lock);
    
    int idx = find_entry_index(store, key);
    if (idx < 0) {
        idcu_mutex_unlock(&store->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    
    if (value_size) {
        *value_size = store->entries[idx].value_size;
    }
    
    if (buffer_size < store->entries[idx].value_size) {
        idcu_mutex_unlock(&store->lock);
        return IDCU_ERR_BUFFER_TOO_SMALL;
    }
    
    memcpy(buffer, store->entries[idx].value, store->entries[idx].value_size);
    idcu_mutex_unlock(&store->lock);
    return IDCU_ERR_OK;
}

int idcu_kvstore_get_string(idcu_KVStore* store, const char* key, char* buffer, size_t buffer_size) {
    size_t value_size;
    int ret = idcu_kvstore_get(store, key, buffer, buffer_size, &value_size);
    if (ret == IDCU_ERR_OK && buffer_size > 0) {
        buffer[buffer_size - 1] = '\0';
    }
    return ret;
}

int idcu_kvstore_get_int(idcu_KVStore* store, const char* key, int64_t* out_value) {
    if (!out_value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t value_size;
    int ret = idcu_kvstore_get(store, key, out_value, sizeof(*out_value), &value_size);
    if (ret == IDCU_ERR_OK && value_size != sizeof(*out_value)) {
        return IDCU_ERR_INVALID_ARG;
    }
    return ret;
}

int idcu_kvstore_get_double(idcu_KVStore* store, const char* key, double* out_value) {
    if (!out_value) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    size_t value_size;
    int ret = idcu_kvstore_get(store, key, out_value, sizeof(*out_value), &value_size);
    if (ret == IDCU_ERR_OK && value_size != sizeof(*out_value)) {
        return IDCU_ERR_INVALID_ARG;
    }
    return ret;
}

int idcu_kvstore_remove(idcu_KVStore* store, const char* key) {
    if (!store || !store->initialized || !key) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&store->lock);
    
    int idx = find_entry_index(store, key);
    if (idx < 0) {
        idcu_mutex_unlock(&store->lock);
        return IDCU_ERR_NOT_FOUND;
    }
    
    // 释放值内存
    if (store->entries[idx].value) {
        free(store->entries[idx].value);
    }
    
    // 将后面的条目前移
    for (uint32_t i = idx; i < store->entry_count - 1; i++) {
        store->entries[i] = store->entries[i + 1];
    }
    
    store->entry_count--;
    store->dirty = 1;
    idcu_mutex_unlock(&store->lock);
    return IDCU_ERR_OK;
}

int idcu_kvstore_sync(idcu_KVStore* store) {
    if (!store || !store->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&store->lock);
    
    int ret = IDCU_ERR_OK;
    if (store->dirty) {
        ret = save_to_json(store);
        if (ret == IDCU_ERR_OK) {
            store->dirty = 0;
        }
    }
    
    idcu_mutex_unlock(&store->lock);
    return ret;
}

int idcu_kvstore_clear(idcu_KVStore* store) {
    if (!store || !store->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&store->lock);
    
    // 释放所有值
    for (uint32_t i = 0; i < store->entry_count; i++) {
        if (store->entries[i].value) {
            free(store->entries[i].value);
        }
    }
    
    store->entry_count = 0;
    store->dirty = 1;
    
    idcu_mutex_unlock(&store->lock);
    return IDCU_ERR_OK;
}

uint32_t idcu_kvstore_count(idcu_KVStore* store) {
    if (!store || !store->initialized) {
        return 0;
    }
    return store->entry_count;
}

// ========== Cache API ==========
void idcu_kvstore_set_cache_config(idcu_KVStore* store, const idcu_KVCacheConfig* config) {
    if (!store || !store->initialized || !config) {
        return;
    }
    store->cache_config = *config;
}

void idcu_kvstore_enable_cache(idcu_KVStore* store, int enabled) {
    if (!store || !store->initialized) {
        return;
    }
    store->cache_config.enabled = enabled;
    if (!enabled) {
        idcu_kvstore_clear_cache(store);
    }
}

void idcu_kvstore_clear_cache(idcu_KVStore* store) {
    if (!store || !store->initialized) {
        return;
    }
    
    idcu_mutex_lock(&store->lock);
    
    while (store->cache_head) {
        cache_remove_entry(store, store->cache_head);
    }
    
    idcu_mutex_unlock(&store->lock);
}

size_t idcu_kvstore_get_cache_hit_count(idcu_KVStore* store) {
    (void)store;
    return g_cache_hits;
}

size_t idcu_kvstore_get_cache_miss_count(idcu_KVStore* store) {
    (void)store;
    return g_cache_misses;
}

// ========== Compression API ==========
void idcu_kvstore_set_compression(idcu_KVStore* store, int enabled, idcu_StorageCompressionLevel level) {
    if (!store || !store->initialized) {
        return;
    }
    store->compression_enabled = enabled;
    store->compression_level = level;
}

// ========== SQLite数据库API（简化版）==========
int idcu_sqlite_init(idcu_SQLiteDB* db, const char* path) {
    if (!db || !path) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(db, 0, sizeof(*db));
    strncpy(db->path, path, IDCU_STORAGE_PATH_MAX - 1);
    db->path[IDCU_STORAGE_PATH_MAX - 1] = '\0';
    
    idcu_mutex_init(&db->lock);
    db->db_handle = NULL; // 简化版不实际连接SQLite
    db->in_transaction = 0;
    db->initialized = 1;
    
    return IDCU_ERR_OK;
}

void idcu_sqlite_destroy(idcu_SQLiteDB* db) {
    if (!db || !db->initialized) {
        return;
    }
    
    idcu_mutex_lock(&db->lock);
    
    idcu_Mutex lock_copy = db->lock;
    db->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_sqlite_execute(idcu_SQLiteDB* db, const char* sql) {
    (void)sql;
    if (!db || !db->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    return IDCU_ERR_OK; // 简化版
}

int idcu_sqlite_begin_transaction(idcu_SQLiteDB* db) {
    if (!db || !db->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&db->lock);
    db->in_transaction = 1;
    idcu_mutex_unlock(&db->lock);
    return IDCU_ERR_OK;
}

int idcu_sqlite_commit(idcu_SQLiteDB* db) {
    if (!db || !db->initialized || !db->in_transaction) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&db->lock);
    db->in_transaction = 0;
    idcu_mutex_unlock(&db->lock);
    return IDCU_ERR_OK;
}

int idcu_sqlite_rollback(idcu_SQLiteDB* db) {
    if (!db || !db->initialized || !db->in_transaction) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    idcu_mutex_lock(&db->lock);
    db->in_transaction = 0;
    idcu_mutex_unlock(&db->lock);
    return IDCU_ERR_OK;
}
