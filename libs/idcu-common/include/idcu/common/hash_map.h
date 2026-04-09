#ifndef IDCU_COMMON_HASH_MAP_H
#define IDCU_COMMON_HASH_MAP_H

#include "error_code.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct idcu_HashMapEntry
{
    char* key;
    void* value;
    struct idcu_HashMapEntry* next;
} idcu_HashMapEntry;

typedef struct
{
    idcu_HashMapEntry** buckets;
    size_t bucket_count;
    size_t size;
    size_t value_size;
    void (*value_dtor)(void*);
} idcu_HashMap;

typedef struct
{
    idcu_HashMap* map;
    size_t bucket_index;
    idcu_HashMapEntry* current_entry;
} idcu_HashMapIterator;

int  idcu_hash_map_init(idcu_HashMap* map, size_t bucket_count, size_t value_size);
int  idcu_hash_map_init_with_dtor(idcu_HashMap* map, size_t bucket_count, size_t value_size,
                                   void (*value_dtor)(void*));
void idcu_hash_map_destroy(idcu_HashMap* map);

int  idcu_hash_map_put(idcu_HashMap* map, const char* key, const void* value);
int  idcu_hash_map_get(const idcu_HashMap* map, const char* key, void* out_value);
int  idcu_hash_map_remove(idcu_HashMap* map, const char* key);
bool idcu_hash_map_contains(const idcu_HashMap* map, const char* key);

size_t idcu_hash_map_size(const idcu_HashMap* map);
bool   idcu_hash_map_empty(const idcu_HashMap* map);
void   idcu_hash_map_clear(idcu_HashMap* map);

void idcu_hash_map_iterator_init(idcu_HashMapIterator* iter, idcu_HashMap* map);
bool idcu_hash_map_iterator_next(idcu_HashMapIterator* iter, const char** out_key, void** out_value);

#define IDCU_HASH_MAP_FOR_EACH(map, key_var, value_var, iter)                          \
    for (idcu_hash_map_iterator_init(&(iter), (map));                                    \
         idcu_hash_map_iterator_next(&(iter), &(key_var), (void**)&(value_var));)

#ifdef __cplusplus
}
#endif

#endif
