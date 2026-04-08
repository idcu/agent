#ifndef IDCU_COMMON_HASH_MAP_H
#define IDCU_COMMON_HASH_MAP_H

#include "error_code.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct idcu_HashMapEntry idcu_HashMapEntry;

    struct idcu_HashMapEntry
    {
        char*              key;
        void*              value;
        idcu_HashMapEntry* next;
    };

    typedef struct
    {
        idcu_HashMapEntry** buckets;
        size_t              bucket_count;
        size_t              size;
        size_t              value_size;
        void (*value_dtor)(void*);
    } idcu_HashMap;

    typedef struct
    {
        idcu_HashMap*      map;
        size_t             bucket_index;
        idcu_HashMapEntry* current_entry;
    } idcu_HashMapIterator;

    int  idcu_hash_map_init(idcu_HashMap* map, size_t value_size, size_t initial_buckets);
    int  idcu_hash_map_init_with_dtor(idcu_HashMap* map, size_t value_size, size_t initial_buckets,
                                      void (*value_dtor)(void*));
    void idcu_hash_map_destroy(idcu_HashMap* map);

    int  idcu_hash_map_insert(idcu_HashMap* map, const char* key, const void* value);
    int  idcu_hash_map_get(const idcu_HashMap* map, const char* key, void* out_value);
    bool idcu_hash_map_contains(const idcu_HashMap* map, const char* key);
    int  idcu_hash_map_remove(idcu_HashMap* map, const char* key);
    void idcu_hash_map_clear(idcu_HashMap* map);

    size_t idcu_hash_map_size(const idcu_HashMap* map);
    bool   idcu_hash_map_empty(const idcu_HashMap* map);

    void idcu_hash_map_iter_init(idcu_HashMapIterator* iter, idcu_HashMap* map);
    bool idcu_hash_map_iter_next(idcu_HashMapIterator* iter, const char** out_key,
                                 void** out_value);
    void idcu_hash_map_iter_destroy(idcu_HashMapIterator* iter);

#define IDCU_HASH_MAP_FOR_EACH(map, key_var, value_var, type)                                    \
    for (idcu_HashMapIterator _iter = {0}, *_p_iter = &_iter;                                    \
         (idcu_hash_map_iter_init(_p_iter, map), 1);                                             \
         idcu_hash_map_iter_destroy(_p_iter), _p_iter = NULL)                                    \
        for (const char* key_var = NULL;                                                         \
             key_var || (idcu_hash_map_iter_next(_p_iter, &key_var, (void**)&value_var) &&       \
                         (key_var = (void*)(uintptr_t)1));)                                      \
            for (type* value_var = NULL; value_var || (key_var = NULL, 1); value_var = (type*)1) \
                if (key_var)                                                                     \
                {                                                                                \
                    idcu_hash_map_iter_next(_p_iter, &key_var, (void**)&value_var);              \
                }                                                                                \
                else

#ifdef __cplusplus
}
#endif

#endif
