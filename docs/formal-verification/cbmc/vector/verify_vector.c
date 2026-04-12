/**
 * @file verify_vector.c
 * @brief CBMC 形式化验证 - 动态数组（Vector）
 *
 * 验证属性:
 * 1. 大小不变式: size <= capacity
 * 2. 数据指针有效性: data != NULL 当 capacity > 0
 * 3. 索引有效性: 0 <= index < size 时访问有效
 * 4. 内存安全: 所有内存操作都是安全的
 * 5. 扩容正确性: 扩容后 capacity 正确增长
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define IDCU_ERR_OK 0
#define IDCU_ERR_INVALID_PARAM 1
#define IDCU_ERR_NO_MEMORY 2

#define IDCU_VECTOR_DEFAULT_CAPACITY 8
#define IDCU_VECTOR_GROWTH_FACTOR 2

typedef struct
{
    void** data;
    size_t size;
    size_t capacity;
    size_t element_size;
    void (*element_dtor)(void*);
} idcu_Vector;

static int idcu_vector_init(idcu_Vector* vec, size_t element_size, size_t initial_capacity)
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

static void idcu_vector_destroy(idcu_Vector* vec)
{
    if (!vec)
        return;

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

static int idcu_vector_push_back(idcu_Vector* vec, const void* element)
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

static int idcu_vector_pop_back(idcu_Vector* vec, void* out_element)
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

static void* idcu_vector_get(const idcu_Vector* vec, size_t index)
{
    if (!vec || index >= vec->size) {
        return NULL;
    }
    return vec->data[index];
}

static size_t idcu_vector_size(const idcu_Vector* vec) { return vec ? vec->size : 0; }
static size_t idcu_vector_capacity(const idcu_Vector* vec) { return vec ? vec->capacity : 0; }
static bool idcu_vector_empty(const idcu_Vector* vec) { return vec ? (vec->size == 0) : true; }

bool verify_vector_invariant(const idcu_Vector* vec)
{
    if (!vec) return true;

    if (vec->size > vec->capacity) {
        return false;
    }

    if (vec->capacity > 0 && vec->data == NULL) {
        return false;
    }

    if (vec->capacity == 0 && vec->data != NULL) {
        return false;
    }

    if (vec->element_size == 0) {
        return false;
    }

    return true;
}

void test_vector_init()
{
    idcu_Vector vec;
    int ret = idcu_vector_init(&vec, sizeof(int), 0);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "初始化应该成功");
    __CPROVER_assert(vec.data != NULL, "data 不应该为 NULL");
    __CPROVER_assert(vec.size == 0, "size 应该为 0");
    __CPROVER_assert(vec.capacity == IDCU_VECTOR_DEFAULT_CAPACITY, "capacity 应该为默认值");
    __CPROVER_assert(verify_vector_invariant(&vec), "向量不变式应该成立");
    
    idcu_vector_destroy(&vec);
}

void test_vector_push_back()
{
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);
    
    __CPROVER_assert(verify_vector_invariant(&vec), "初始状态不变式成立");
    
    int value = 42;
    int ret = idcu_vector_push_back(&vec, &value);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "push_back 应该成功");
    __CPROVER_assert(vec.size == 1, "size 应该为 1");
    __CPROVER_assert(verify_vector_invariant(&vec), "push_back 后不变式成立");
    
    int* ptr = (int*)idcu_vector_get(&vec, 0);
    __CPROVER_assert(ptr != NULL, "获取的指针不应该为 NULL");
    __CPROVER_assert(*ptr == 42, "存储的值应该正确");
    
    idcu_vector_destroy(&vec);
}

void test_vector_grow()
{
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 2);
    
    __CPROVER_assert(vec.capacity == 2, "初始 capacity 应该为 2");
    
    int v1 = 1, v2 = 2, v3 = 3;
    
    idcu_vector_push_back(&vec, &v1);
    idcu_vector_push_back(&vec, &v2);
    
    __CPROVER_assert(vec.size == 2, "size 应该为 2");
    __CPROVER_assert(vec.capacity == 2, "capacity 应该仍为 2");
    
    idcu_vector_push_back(&vec, &v3);
    
    __CPROVER_assert(vec.size == 3, "size 应该为 3");
    __CPROVER_assert(vec.capacity == 4, "capacity 应该增长到 4");
    __CPROVER_assert(verify_vector_invariant(&vec), "扩容后不变式成立");
    
    idcu_vector_destroy(&vec);
}

void test_vector_pop_back()
{
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);
    
    int value = 42;
    idcu_vector_push_back(&vec, &value);
    
    int out;
    int ret = idcu_vector_pop_back(&vec, &out);
    
    __CPROVER_assert(ret == IDCU_ERR_OK, "pop_back 应该成功");
    __CPROVER_assert(out == 42, "弹出的值应该正确");
    __CPROVER_assert(vec.size == 0, "size 应该为 0");
    __CPROVER_assert(verify_vector_invariant(&vec), "pop_back 后不变式成立");
    
    idcu_vector_destroy(&vec);
}

void test_vector_multiple_operations()
{
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);
    
    __CPROVER_assert(verify_vector_invariant(&vec), "初始状态不变式成立");
    
    int i;
    for (i = 0; i < 10; i++) {
        int v = i * 10;
        idcu_vector_push_back(&vec, &v);
        __CPROVER_assert(verify_vector_invariant(&vec), "每次 push_back 后不变式成立");
    }
    
    __CPROVER_assert(vec.size == 10, "size 应该为 10");
    
    for (i = 0; i < 5; i++) {
        int out;
        idcu_vector_pop_back(&vec, &out);
        __CPROVER_assert(verify_vector_invariant(&vec), "每次 pop_back 后不变式成立");
    }
    
    __CPROVER_assert(vec.size == 5, "size 应该为 5");
    
    idcu_vector_destroy(&vec);
}

int main()
{
    test_vector_init();
    test_vector_push_back();
    test_vector_grow();
    test_vector_pop_back();
    test_vector_multiple_operations();
    
    return 0;
}
