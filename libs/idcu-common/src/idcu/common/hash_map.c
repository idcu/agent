#include "idcu/common/hash_map.h"
#include <stdlib.h>
#include <string.h>

#define IDCU_HASH_MAP_DEFAULT_BUCKETS 16
#define IDCU_HASH_MAP_MAX_LOAD_FACTOR 0.75
#define IDCU_HASH_MAP_GROWTH_FACTOR 2

static inline uint32_t rotl32(uint32_t x, int8_t r) { return (x << r) | (x >> (32 - r)); }

static inline uint32_t fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

static uint32_t idcu_hash_string(const char *str) {
    if (!str || *str == '\0') {
        return 0;
    }

    const uint8_t *data = (const uint8_t *)str;
    size_t len = strlen(str);
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    uint32_t h1 = 0;

    const int nblocks = len / 4;
    for (int i = 0; i < nblocks; i++) {
        uint32_t k1 = ((uint32_t)data[i * 4]) | ((uint32_t)data[i * 4 + 1] << 8) |
                      ((uint32_t)data[i * 4 + 2] << 16) | ((uint32_t)data[i * 4 + 3] << 24);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
    case 3:
        k1 ^= (uint32_t)tail[2] << 16;
        /* fall through */
    case 2:
        k1 ^= (uint32_t)tail[1] << 8;
        /* fall through */
    case 1:
        k1 ^= (uint32_t)tail[0];
        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;
        h1 ^= k1;
        break;
    }

    h1 ^= len;
    h1 = fmix32(h1);
    return h1;
}

static idcu_HashMapEntry *idcu_hash_map_create_entry(const char *key, const void *value,
                                                     size_t value_size) {
    idcu_HashMapEntry *entry = (idcu_HashMapEntry *)malloc(sizeof(idcu_HashMapEntry));
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

static void idcu_hash_map_destroy_entry(idcu_HashMapEntry *entry, void (*value_dtor)(void *)) {
    if (!entry)
        return;
    free(entry->key);
    if (value_dtor) {
        value_dtor(entry->value);
    }
    free(entry->value);
    free(entry);
}

int idcu_hash_map_init(idcu_HashMap *map, size_t value_size, size_t initial_buckets) {
    if (!map || value_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (initial_buckets == 0) {
        initial_buckets = IDCU_HASH_MAP_DEFAULT_BUCKETS;
    }

    map->buckets = (idcu_HashMapEntry **)calloc(initial_buckets, sizeof(idcu_HashMapEntry *));
    if (!map->buckets) {
        return IDCU_ERR_NO_MEMORY;
    }

    map->bucket_count = initial_buckets;
    map->size = 0;
    map->value_size = value_size;
    map->value_dtor = NULL;
    return IDCU_ERR_OK;
}

int idcu_hash_map_init_with_dtor(idcu_HashMap *map, size_t value_size, size_t initial_buckets,
                                 void (*value_dtor)(void *)) {
    int ret = idcu_hash_map_init(map, value_size, initial_buckets);
    if (ret == IDCU_ERR_OK) {
        map->value_dtor = value_dtor;
    }
    return ret;
}

void idcu_hash_map_destroy(idcu_HashMap *map) {
    if (!map)
        return;
    idcu_hash_map_clear(map);
    free(map->buckets);
    map->buckets = NULL;
    map->bucket_count = 0;
    map->size = 0;
}

static int idcu_hash_map_rehash(idcu_HashMap *map) {
    size_t new_bucket_count = map->bucket_count * IDCU_HASH_MAP_GROWTH_FACTOR;
    idcu_HashMapEntry **new_buckets =
        (idcu_HashMapEntry **)calloc(new_bucket_count, sizeof(idcu_HashMapEntry *));
    if (!new_buckets) {
        return IDCU_ERR_NO_MEMORY;
    }

    for (size_t i = 0; i < map->bucket_count; ++i) {
        idcu_HashMapEntry *entry = map->buckets[i];
        while (entry) {
            idcu_HashMapEntry *next = entry->next;
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

int idcu_hash_map_insert(idcu_HashMap *map, const char *key, const void *value) {
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

    idcu_HashMapEntry *existing = map->buckets[index];
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

    idcu_HashMapEntry *new_entry = idcu_hash_map_create_entry(key, value, map->value_size);
    if (!new_entry) {
        return IDCU_ERR_NO_MEMORY;
    }

    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->size++;
    return IDCU_ERR_OK;
}

int idcu_hash_map_get(const idcu_HashMap *map, const char *key, void *out_value) {
    if (!map || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t hash = idcu_hash_string(key);
    size_t index = hash % map->bucket_count;

    idcu_HashMapEntry *entry = map->buckets[index];
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

bool idcu_hash_map_contains(const idcu_HashMap *map, const char *key) {
    return idcu_hash_map_get(map, key, NULL) == IDCU_ERR_OK;
}

int idcu_hash_map_remove(idcu_HashMap *map, const char *key) {
    if (!map || !key) {
        return IDCU_ERR_INVALID_PARAM;
    }

    uint32_t hash = idcu_hash_string(key);
    size_t index = hash % map->bucket_count;

    idcu_HashMapEntry **prev_ptr = &map->buckets[index];
    idcu_HashMapEntry *entry = map->buckets[index];
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

void idcu_hash_map_clear(idcu_HashMap *map) {
    if (!map)
        return;
    for (size_t i = 0; i < map->bucket_count; ++i) {
        idcu_HashMapEntry *entry = map->buckets[i];
        while (entry) {
            idcu_HashMapEntry *next = entry->next;
            idcu_hash_map_destroy_entry(entry, map->value_dtor);
            entry = next;
        }
        map->buckets[i] = NULL;
    }
    map->size = 0;
}

size_t idcu_hash_map_size(const idcu_HashMap *map) { return map ? map->size : 0; }

bool idcu_hash_map_empty(const idcu_HashMap *map) { return map ? (map->size == 0) : true; }

void idcu_hash_map_iter_init(idcu_HashMapIterator *iter, idcu_HashMap *map) {
    if (!iter || !map)
        return;
    iter->map = map;
    iter->bucket_index = 0;
    iter->current_entry = NULL;
}

bool idcu_hash_map_iter_next(idcu_HashMapIterator *iter, const char **out_key, void **out_value) {
    if (!iter || !iter->map)
        return false;

    if (iter->current_entry && iter->current_entry->next) {
        iter->current_entry = iter->current_entry->next;
        if (out_key)
            *out_key = iter->current_entry->key;
        if (out_value)
            *out_value = iter->current_entry->value;
        return true;
    }

    while (iter->bucket_index < iter->map->bucket_count) {
        if (iter->map->buckets[iter->bucket_index]) {
            iter->current_entry = iter->map->buckets[iter->bucket_index];
            iter->bucket_index++;
            if (out_key)
                *out_key = iter->current_entry->key;
            if (out_value)
                *out_value = iter->current_entry->value;
            return true;
        }
        iter->bucket_index++;
    }

    return false;
}

void idcu_hash_map_iter_destroy(idcu_HashMapIterator *iter) {
    if (!iter)
        return;
    iter->map = NULL;
    iter->bucket_index = 0;
    iter->current_entry = NULL;
}
