#include "idcu/common/hash_map.h"
#include <stdlib.h>
#include <string.h>

#define IDCU_HASH_MAP_DEFAULT_BUCKETS 32
#define IDCU_HASH_MAP_MAX_LOAD_FACTOR 0.7f

#define ROTL32(x, r) (((x) << (r)) | ((x) >> (32 - (r))))

static uint32_t idcu_murmur3_32(const void* key, size_t len, uint32_t seed)
{
    const uint8_t* data = (const uint8_t*)key;
    const size_t nblocks = len / 4;
    uint32_t h1 = seed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    
    const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);
    for (size_t i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];
        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;
        h1 ^= k1;
        h1 = ROTL32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }
    
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
    uint32_t k1 = 0;
    switch (len & 3) {
        case 3: k1 ^= (uint32_t)tail[2] << 16;
        case 2: k1 ^= (uint32_t)tail[1] << 8;
        case 1: k1 ^= (uint32_t)tail[0];
                k1 *= c1;
                k1 = ROTL32(k1, 15);
                k1 *= c2;
                h1 ^= k1;
    }
    
    h1 ^= (uint32_t)len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
}

static uint32_t idcu_hash_func(const char* key)
{
    size_t len = strlen(key);
    return idcu_murmur3_32(key, len, 0x9747b28c);
}

static int idcu_hash_map_resize(idcu_HashMap* map, size_t new_bucket_count)
{
    if (!map || new_bucket_count == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_HashMapEntry** new_buckets = (idcu_HashMapEntry**)calloc(new_bucket_count, sizeof(idcu_HashMapEntry*));
    if (!new_buckets) {
        return IDCU_ERR_NO_MEMORY;
    }

    // Rehash all entries
    for (size_t i = 0; i < map->bucket_count; ++i) {
        idcu_HashMapEntry* entry = map->buckets[i];
        while (entry) {
            idcu_HashMapEntry* next = entry->next;
            uint32_t hash = idcu_hash_func(entry->key);
            size_t new_bucket_idx = hash % new_bucket_count;
            entry->next = new_buckets[new_bucket_idx];
            new_buckets[new_bucket_idx] = entry;
            entry = next;
        }
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->bucket_count = new_bucket_count;

    return IDCU_ERR_OK;
}

int idcu_hash_map_init(idcu_HashMap* map, size_t bucket_count, size_t value_size)
{
    if (!map || value_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (bucket_count == 0) {
        bucket_count = IDCU_HASH_MAP_DEFAULT_BUCKETS;
    }

    map->buckets = (idcu_HashMapEntry**)calloc(bucket_count, sizeof(idcu_HashMapEntry*));
    if (!map->buckets) {
        return IDCU_ERR_NO_MEMORY;
    }

    map->bucket_count = bucket_count;
    map->size = 0;
    map->value_size = value_size;
    map->value_dtor = NULL;

    return IDCU_ERR_OK;
}

int idcu_hash_map_init_with_dtor(idcu_HashMap* map, size_t bucket_count, size_t value_size,
                                   void (*value_dtor)(void*))
{
    int ret = idcu_hash_map_init(map, bucket_count, value_size);
    if (ret == IDCU_ERR_OK) {
        map->value_dtor = value_dtor;
    }
    return ret;
}

static void idcu_hash_map_destroy_entry(idcu_HashMap* map, idcu_HashMapEntry* entry)
{
    if (!entry)
        return;

    if (map->value_dtor) {
        map->value_dtor(entry->value);
    }
    free(entry->value);
    free(entry->key);
    free(entry);
}

void idcu_hash_map_destroy(idcu_HashMap* map)
{
    if (!map)
        return;

    for (size_t i = 0; i < map->bucket_count; ++i) {
        idcu_HashMapEntry* entry = map->buckets[i];
        while (entry) {
            idcu_HashMapEntry* next = entry->next;
            idcu_hash_map_destroy_entry(map, entry);
            entry = next;
        }
    }

    free(map->buckets);
    map->buckets = NULL;
    map->bucket_count = 0;
    map->size = 0;
}

int idcu_hash_map_put(idcu_HashMap* map, const char* key, const void* value)
{
    if (!map || !key || !value) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t hash = idcu_hash_func(key);
    size_t bucket_idx = hash % map->bucket_count;

    idcu_HashMapEntry* entry = map->buckets[bucket_idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (map->value_dtor) {
                map->value_dtor(entry->value);
            }
            memcpy(entry->value, value, map->value_size);
            return IDCU_ERR_OK;
        }
        entry = entry->next;
    }

    entry = (idcu_HashMapEntry*)malloc(sizeof(idcu_HashMapEntry));
    if (!entry) {
        return IDCU_ERR_NO_MEMORY;
    }

    entry->key = strdup(key);
    if (!entry->key) {
        free(entry);
        return IDCU_ERR_NO_MEMORY;
    }

    entry->value = malloc(map->value_size);
    if (!entry->value) {
        free(entry->key);
        free(entry);
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(entry->value, value, map->value_size);

    entry->next = map->buckets[bucket_idx];
    map->buckets[bucket_idx] = entry;
    map->size++;

    // Check if we need to resize
    float load_factor = (float)map->size / (float)map->bucket_count;
    if (load_factor > IDCU_HASH_MAP_MAX_LOAD_FACTOR) {
        int ret = idcu_hash_map_resize(map, map->bucket_count * 2);
        if (ret != IDCU_ERR_OK) {
            // Resize failed, but we still added the entry
            return IDCU_ERR_OK;
        }
    }

    return IDCU_ERR_OK;
}

int idcu_hash_map_get(const idcu_HashMap* map, const char* key, void* out_value)
{
    if (!map || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t hash = idcu_hash_func(key);
    size_t bucket_idx = hash % map->bucket_count;

    idcu_HashMapEntry* entry = map->buckets[bucket_idx];
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

int idcu_hash_map_remove(idcu_HashMap* map, const char* key)
{
    if (!map || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t hash = idcu_hash_func(key);
    size_t bucket_idx = hash % map->bucket_count;

    idcu_HashMapEntry** prev = &map->buckets[bucket_idx];
    idcu_HashMapEntry* entry = *prev;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            *prev = entry->next;
            idcu_hash_map_destroy_entry(map, entry);
            map->size--;
            return IDCU_ERR_OK;
        }
        prev = &entry->next;
        entry = entry->next;
    }

    return IDCU_ERR_NOT_FOUND;
}

bool idcu_hash_map_contains(const idcu_HashMap* map, const char* key)
{
    if (!map || !key) {
        return false;
    }

    uint32_t hash = idcu_hash_func(key);
    size_t bucket_idx = hash % map->bucket_count;

    idcu_HashMapEntry* entry = map->buckets[bucket_idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return true;
        }
        entry = entry->next;
    }

    return false;
}

size_t idcu_hash_map_size(const idcu_HashMap* map) { return map ? map->size : 0; }
bool idcu_hash_map_empty(const idcu_HashMap* map) { return map ? (map->size == 0) : true; }

void idcu_hash_map_clear(idcu_HashMap* map)
{
    if (!map)
        return;

    for (size_t i = 0; i < map->bucket_count; ++i) {
        idcu_HashMapEntry* entry = map->buckets[i];
        while (entry) {
            idcu_HashMapEntry* next = entry->next;
            idcu_hash_map_destroy_entry(map, entry);
            entry = next;
        }
        map->buckets[i] = NULL;
    }

    map->size = 0;
}

void idcu_hash_map_iterator_init(idcu_HashMapIterator* iter, idcu_HashMap* map)
{
    if (!iter || !map) {
        return;
    }
    iter->map = map;
    iter->bucket_index = 0;
    iter->current_entry = NULL;
}

bool idcu_hash_map_iterator_next(idcu_HashMapIterator* iter, const char** out_key, void** out_value)
{
    if (!iter || !iter->map) {
        return false;
    }

    if (iter->current_entry && iter->current_entry->next) {
        iter->current_entry = iter->current_entry->next;
        if (out_key) {
            *out_key = iter->current_entry->key;
        }
        if (out_value) {
            *out_value = iter->current_entry->value;
        }
        return true;
    }

    for (size_t i = iter->bucket_index; i < iter->map->bucket_count; ++i) {
        if (iter->map->buckets[i]) {
            iter->current_entry = iter->map->buckets[i];
            iter->bucket_index = i + 1;
            if (out_key) {
                *out_key = iter->current_entry->key;
            }
            if (out_value) {
                *out_value = iter->current_entry->value;
            }
            return true;
        }
    }

    return false;
}

void* idcu_hash_map_get_ptr(const idcu_HashMap* map, const char* key)
{
    if (!map || !key) {
        return NULL;
    }

    uint32_t hash = idcu_hash_func(key);
    size_t bucket_idx = hash % map->bucket_count;

    idcu_HashMapEntry* entry = map->buckets[bucket_idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}
