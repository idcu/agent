#ifndef IDCU_COMMON_VECTOR_H
#define IDCU_COMMON_VECTOR_H

#include "error_code.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        void** data;
        size_t size;
        size_t capacity;
        size_t element_size;
        void (*element_dtor)(void*);
    } idcu_Vector;

    int  idcu_vector_init(idcu_Vector* vec, size_t element_size, size_t initial_capacity);
    int  idcu_vector_init_with_dtor(idcu_Vector* vec, size_t element_size, size_t initial_capacity,
                                    void (*element_dtor)(void*));
    void idcu_vector_destroy(idcu_Vector* vec);

    int idcu_vector_push_back(idcu_Vector* vec, const void* element);
    int idcu_vector_pop_back(idcu_Vector* vec, void* out_element);
    int idcu_vector_insert(idcu_Vector* vec, size_t index, const void* element);
    int idcu_vector_remove(idcu_Vector* vec, size_t index);

    void* idcu_vector_get(const idcu_Vector* vec, size_t index);
    int   idcu_vector_set(idcu_Vector* vec, size_t index, const void* element);

    size_t idcu_vector_size(const idcu_Vector* vec);
    size_t idcu_vector_capacity(const idcu_Vector* vec);
    bool   idcu_vector_empty(const idcu_Vector* vec);
    void   idcu_vector_clear(idcu_Vector* vec);

    int idcu_vector_reserve(idcu_Vector* vec, size_t new_capacity);
    int idcu_vector_resize(idcu_Vector* vec, size_t new_size, const void* default_value);

#define IDCU_VECTOR_FOR_EACH(vec, type, var, index)                \
    for (size_t index = 0; index < idcu_vector_size(vec); ++index) \
        for (type* var = (type*)idcu_vector_get(vec, index); var; var = NULL)

#ifdef __cplusplus
}
#endif

#endif
