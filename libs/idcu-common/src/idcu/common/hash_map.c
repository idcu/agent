#include "idcu/common/hash_map.h"
#include <stdlib.h>
#include <string.h>

#define IDCU_HASH_MAP_DEFAULT_BUCKETS 16
#define IDCU_HASH_MAP_MAX_LOAD_FACTOR 0.75
#define IDCU_HASH_MAP_GROWTH_FACTOR 2

static uint32_t idcu_hash_string(const char* str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static idcu_HashMapEntry* idcu_hash_map_create_entry(const char* key, const void* value, size_t value_size)
{
    idcu_HashMapEntry* entry = (idcu_HashMapEntry*)malloc(sizeof(idcu_HashMapEntry));
    if (!entry) {
        return NULL;
    }

    entry->key = strdup(key);
    if (!entry->key) {
        free(entry);
        return NULL;
    }

    entry->value = malloc(value_size);
    if (!entry->value) {
        free(entry->key);
        free(entry);
        return NULL;
    }

    memcpy(entry->value, value, value_size);
    entry->next = NULL;
    return entry;
}

static void idcu_hash_map_destroy_entry(idcu_HashMapEntry* entry, void (*value_dtor)(void*))
{
    if (!entry) return;
    free(entry->key);
    if (value_dtor) {
        value_dtor(entry->value);
    }
    free(entry->value);
    free(entry);
}

int idcu_hash_map_init(idcu_HashMap* map, size_t value_size, size_t initial_buckets)
{
    if (!map || value_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (initial_buckets == 0) {
        initial_buckets = IDCU_HASH_MAP_DEFAULT_BUCKETS;
    }

    map->buckets = (idcu_HashMapEntry**)calloc(initial_buckets, sizeof(idcu_HashMapEntry*));
    if (!map->buckets) {
        return IDCU_ERR_NO_MEMORY;
    }

    map->bucket_count = initial_buckets;
    map->size = 0;
    map->value_size = value_size;
    map->value_dtor = NULL;
    return IDCU_ERR_OK;
}

int idcu_hash_map_init_with_dtor(idcu_HashMap* map, size_t value_size, size_t initial_buckets,
                                   void (*value_dtor)(void*))
{
    int ret = idcu_hash_map_init(map, value_size, initial_buckets);
    if (ret == IDCU_ERR_OK) {
        map->value_dtor = value_dtor;
    }
    return ret;
}

void idcu_hash_map_destroy(idcu_HashMap* map)
{
    if (!map) return;
    idcu_hash_map_clear(map);
    free(map->buckets);
    map->buckets = NULL;
    map->bucket_count = 0;
    map->size = 0;
}

static int idcu_hash_map_rehash(idcu_HashMap* map)
{
    size_t new_bucket_count = map->bucket_count * IDCU_HASH_MAP_GROWTH_FACTOR;
    idcu_HashMapEntry** new_buckets = (idcu_HashMapEntry**)calloc(new_bucket_count, sizeof(idcu_HashMapEntry*));
    if (!new_buckets) {
        return IDCU_ERR_NO_MEMORY;
    }

    for (size_t i = 0; i < map->bucket_count; ++i) {
        idcu_HashMapEntry* entry = map->buckets[i];
        while (entry) {
            idcu_HashMapEntry* next = entry->next;
            uint32_t hash = idcu_hash_string(entry->key);
            size_t new_index = hash % new_bucket_count;
            entry->next = new_buckets[new_index];
            new_buckets[new_index] = entry;
            entry = next;
        }
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->bucket_count = new_bucket_count;
    return IDCU_ERR_OK;
}

int idcu_hash_map_insert(idcu_HashMap* map, const char* key, const void* value)
{
    if (!map || !key || !value) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if ((double)map->size / map->bucket_count > IDCU_HASH_MAP_MAX_LOAD_FACTOR) {
        int ret = idcu_hash_map_rehash(map);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }

    uint32_t hash = idcu_hash_string(key);
    size_t index = hash % map->bucket_count;

    idcu_HashMapEntry* existing = map->buckets[index];
    while (existing) {
        if (strcmp(existing->key, key) == 0) {
            if (map->value_dtor) {
                map->value_dtor(existing->value);
            }
            memcpy(existing->value, value, map->value_size);
            return IDCU_ERR_OK;
        }
        existing = existing->next;
    }

    idcu_HashMapEntry* new_entry = idcu_hash_map_create_entry(key, value, map->value_size);
    if (!new_entry) {
        return IDCU_ERR_NO_MEMORY;
    }

    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->size++;
    return IDCU_ERR_OK;
}

int idcu_hash_map_get(const idcu_HashMap* map, const char* key, void* out_value)
{
    if (!map || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t hash = idcu_hash_string(key);
    size_t index = hash % map->bucket_count;

    idcu_HashMapEntry* entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (out_value) {
                memcpy(out_value, entry->value, map->value_size);
            }
            return IDCU_ERR_OK;
        }
        entry = entry->next;
    }

    return IDCU_ERR_NOT_FOUND;
}

bool idcu_hash_map_contains(const idcu_HashMap* map, const char* key)
{
    return idcu_hash_map_get(map, key, NULL) == IDCU_ERR_OK;
}

int idcu_hash_map_remove(idcu_HashMap* map, const char* key)
{
    if (!map || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t hash = idcu_hash_string(key);
    size_t index = hash % map->bucket_count;

    idcu_HashMapEntry** prev_ptr = &map->buckets[index];
    idcu_HashMapEntry* entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            *prev_ptr = entry->next;
            idcu_hash_map_destroy_entry(entry, map->value_dtor);
            map->size--;
            return IDCU_ERR_OK;
        }
        prev_ptr = &entry->next;
        entry = entry->next;
    }

    return IDCU_ERR_NOT_FOUND;
}

void idcu_hash_map_clear(idcu_HashMap* map)
{
    if (!map) return;
    for (size_t i = 0; i < map->bucket_count; ++i) {
        idcu_HashMapEntry* entry = map->buckets[i];
        while (entry) {
            idcu_HashMapEntry* next = entry->next;
            idcu_hash_map_destroy_entry(entry, map->value_dtor);
            entry = next;
        }
        map->buckets[i] = NULL;
    }
    map->size = 0;
}

size_t idcu_hash_map_size(const idcu_HashMap* map)
{
    return map ? map->size : 0;
}

bool idcu_hash_map_empty(const idcu_HashMap* map)
{
    return map ? (map->size == 0) : true;
}

void idcu_hash_map_iter_init(idcu_HashMapIterator* iter, idcu_HashMap* map)
{
    if (!iter || !map) return;
    iter->map = map;
    iter->bucket_index = 0;
    iter->current_entry = NULL;
}

bool idcu_hash_map_iter_next(idcu_HashMapIterator* iter, const char** out_key, void** out_value)
{
    if (!iter || !iter->map) return false;

    if (iter->current_entry && iter->current_entry->next) {
        iter->current_entry = iter->current_entry->next;
        if (out_key) *out_key = iter->current_entry->key;
        if (out_value) *out_value = iter->current_entry->value;
        return true;
    }

    while (iter->bucket_index < iter->map->bucket_count) {
        if (iter->map->buckets[iter->bucket_index]) {
            iter->current_entry = iter->map->buckets[iter->bucket_index];
            iter->bucket_index++;
            if (out_key) *out_key = iter->current_entry->key;
            if (out_value) *out_value = iter->current_entry->value;
            return true;
        }
        iter->bucket_index++;
    }

    return false;
}

void idcu_hash_map_iter_destroy(idcu_HashMapIterator* iter)
{
    if (!iter) return;
    iter->map = NULL;
    iter->bucket_index = 0;
    iter->current_entry = NULL;
}
