#include "idcu/common/vector.h"
#include <stdlib.h>
#include <string.h>

#define IDCU_VECTOR_DEFAULT_CAPACITY 8
#define IDCU_VECTOR_GROWTH_FACTOR 2

int idcu_vector_init(idcu_Vector* vec, size_t element_size, size_t initial_capacity)
{
    if (!vec || element_size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (initial_capacity == 0) {
        initial_capacity = IDCU_VECTOR_DEFAULT_CAPACITY;
    }

    vec->data = (void**)malloc(initial_capacity * sizeof(void*));
    if (!vec->data) {
        return IDCU_ERR_NO_MEMORY;
    }

    vec->size = 0;
    vec->capacity = initial_capacity;
    vec->element_size = element_size;
    vec->element_dtor = NULL;

    return IDCU_ERR_OK;
}

int idcu_vector_init_with_dtor(idcu_Vector* vec, size_t element_size, size_t initial_capacity,
                                void (*element_dtor)(void*))
{
    int ret = idcu_vector_init(vec, element_size, initial_capacity);
    if (ret == IDCU_ERR_OK) {
        vec->element_dtor = element_dtor;
    }
    return ret;
}

void idcu_vector_destroy(idcu_Vector* vec)
{
    if (!vec) return;

    if (vec->element_dtor) {
        for (size_t i = 0; i < vec->size; ++i) {
            vec->element_dtor(vec->data[i]);
        }
    }

    for (size_t i = 0; i < vec->size; ++i) {
        free(vec->data[i]);
    }

    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

static int idcu_vector_grow(idcu_Vector* vec)
{
    size_t new_capacity = vec->capacity * IDCU_VECTOR_GROWTH_FACTOR;
    void** new_data = (void**)realloc(vec->data, new_capacity * sizeof(void*));
    if (!new_data) {
        return IDCU_ERR_NO_MEMORY;
    }
    vec->data = new_data;
    vec->capacity = new_capacity;
    return IDCU_ERR_OK;
}

int idcu_vector_push_back(idcu_Vector* vec, const void* element)
{
    if (!vec || !element) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (vec->size >= vec->capacity) {
        int ret = idcu_vector_grow(vec);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }

    void* new_element = malloc(vec->element_size);
    if (!new_element) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(new_element, element, vec->element_size);
    vec->data[vec->size++] = new_element;

    return IDCU_ERR_OK;
}

int idcu_vector_pop_back(idcu_Vector* vec, void* out_element)
{
    if (!vec || vec->size == 0) {
        return IDCU_ERR_INVALID_PARAM;
    }

    void* element = vec->data[--vec->size];
    if (out_element) {
        memcpy(out_element, element, vec->element_size);
    }

    if (vec->element_dtor) {
        vec->element_dtor(element);
    }
    free(element);

    return IDCU_ERR_OK;
}

int idcu_vector_insert(idcu_Vector* vec, size_t index, const void* element)
{
    if (!vec || !element || index > vec->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (vec->size >= vec->capacity) {
        int ret = idcu_vector_grow(vec);
        if (ret != IDCU_ERR_OK) {
            return ret;
        }
    }

    for (size_t i = vec->size; i > index; --i) {
        vec->data[i] = vec->data[i - 1];
    }

    void* new_element = malloc(vec->element_size);
    if (!new_element) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(new_element, element, vec->element_size);
    vec->data[index] = new_element;
    vec->size++;

    return IDCU_ERR_OK;
}

int idcu_vector_remove(idcu_Vector* vec, size_t index)
{
    if (!vec || index >= vec->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    void* element = vec->data[index];

    if (vec->element_dtor) {
        vec->element_dtor(element);
    }
    free(element);

    for (size_t i = index; i < vec->size - 1; ++i) {
        vec->data[i] = vec->data[i + 1];
    }
    vec->size--;

    return IDCU_ERR_OK;
}

void* idcu_vector_get(const idcu_Vector* vec, size_t index)
{
    if (!vec || index >= vec->size) {
        return NULL;
    }
    return vec->data[index];
}

int idcu_vector_set(idcu_Vector* vec, size_t index, const void* element)
{
    if (!vec || !element || index >= vec->size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    void* old_element = vec->data[index];
    if (vec->element_dtor) {
        vec->element_dtor(old_element);
    }
    free(old_element);

    void* new_element = malloc(vec->element_size);
    if (!new_element) {
        return IDCU_ERR_NO_MEMORY;
    }
    memcpy(new_element, element, vec->element_size);
    vec->data[index] = new_element;

    return IDCU_ERR_OK;
}

size_t idcu_vector_size(const idcu_Vector* vec)
{
    return vec ? vec->size : 0;
}

size_t idcu_vector_capacity(const idcu_Vector* vec)
{
    return vec ? vec->capacity : 0;
}

bool idcu_vector_empty(const idcu_Vector* vec)
{
    return vec ? (vec->size == 0) : true;
}

void idcu_vector_clear(idcu_Vector* vec)
{
    if (!vec) return;

    if (vec->element_dtor) {
        for (size_t i = 0; i < vec->size; ++i) {
            vec->element_dtor(vec->data[i]);
        }
    }

    for (size_t i = 0; i < vec->size; ++i) {
        free(vec->data[i]);
    }

    vec->size = 0;
}

int idcu_vector_reserve(idcu_Vector* vec, size_t new_capacity)
{
    if (!vec) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (new_capacity <= vec->capacity) {
        return IDCU_ERR_OK;
    }

    void** new_data = (void**)realloc(vec->data, new_capacity * sizeof(void*));
    if (!new_data) {
        return IDCU_ERR_NO_MEMORY;
    }
    vec->data = new_data;
    vec->capacity = new_capacity;
    return IDCU_ERR_OK;
}

int idcu_vector_resize(idcu_Vector* vec, size_t new_size, const void* default_value)
{
    if (!vec) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (new_size < vec->size) {
        for (size_t i = new_size; i < vec->size; ++i) {
            if (vec->element_dtor) {
                vec->element_dtor(vec->data[i]);
            }
            free(vec->data[i]);
        }
        vec->size = new_size;
    } else if (new_size > vec->size) {
        for (size_t i = vec->size; i < new_size; ++i) {
            int ret = idcu_vector_push_back(vec, default_value);
            if (ret != IDCU_ERR_OK) {
                return ret;
            }
        }
    }

    return IDCU_ERR_OK;
}
