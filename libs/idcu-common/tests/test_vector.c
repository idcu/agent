#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(cond, msg)                                                                     \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "FAIL: %s\n", msg);                                                    \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

static int test_vector_init_destroy(void) {
    idcu_Vector vec;
    int ret = idcu_vector_init(&vec, sizeof(int), 4);
    TEST_ASSERT(ret == IDCU_ERR_OK, "vector_init should succeed");

    idcu_vector_destroy(&vec);
    printf("PASS: test_vector_init_destroy\n");
    return 0;
}

static int test_vector_push_back(void) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);

    for (int i = 0; i < 10; i++) {
        int ret = idcu_vector_push_back(&vec, &i);
        TEST_ASSERT(ret == IDCU_ERR_OK, "push_back should succeed");
    }

    size_t size = idcu_vector_size(&vec);
    TEST_ASSERT(size == 10, "size should be 10");

    idcu_vector_destroy(&vec);
    printf("PASS: test_vector_push_back\n");
    return 0;
}

static int test_vector_get_set(void) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);

    int value = 42;
    idcu_vector_push_back(&vec, &value);

    int *out_value = (int *)idcu_vector_get(&vec, 0);
    TEST_ASSERT(out_value != NULL, "get should return non-null");
    TEST_ASSERT(*out_value == 42, "value should be 42");

    int new_value = 100;
    int ret = idcu_vector_set(&vec, 0, &new_value);
    TEST_ASSERT(ret == IDCU_ERR_OK, "set should succeed");

    out_value = (int *)idcu_vector_get(&vec, 0);
    TEST_ASSERT(*out_value == 100, "value should be 100");

    idcu_vector_destroy(&vec);
    printf("PASS: test_vector_get_set\n");
    return 0;
}

static int test_vector_pop_back(void) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);

    int value1 = 42;
    int value2 = 100;
    idcu_vector_push_back(&vec, &value1);
    idcu_vector_push_back(&vec, &value2);

    int out_value;
    int ret = idcu_vector_pop_back(&vec, &out_value);
    TEST_ASSERT(ret == IDCU_ERR_OK, "pop_back should succeed");
    TEST_ASSERT(out_value == 100, "popped value should be 100");

    size_t size = idcu_vector_size(&vec);
    TEST_ASSERT(size == 1, "size should be 1");

    idcu_vector_destroy(&vec);
    printf("PASS: test_vector_pop_back\n");
    return 0;
}

static int test_vector_insert_remove(void) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);

    int value1 = 1, value2 = 2, value3 = 3;
    idcu_vector_push_back(&vec, &value1);
    idcu_vector_push_back(&vec, &value3);

    int ret = idcu_vector_insert(&vec, 1, &value2);
    TEST_ASSERT(ret == IDCU_ERR_OK, "insert should succeed");

    size_t size = idcu_vector_size(&vec);
    TEST_ASSERT(size == 3, "size should be 3");

    int *out_value = (int *)idcu_vector_get(&vec, 1);
    TEST_ASSERT(*out_value == 2, "value at index 1 should be 2");

    ret = idcu_vector_remove(&vec, 1);
    TEST_ASSERT(ret == IDCU_ERR_OK, "remove should succeed");

    size = idcu_vector_size(&vec);
    TEST_ASSERT(size == 2, "size should be 2");

    out_value = (int *)idcu_vector_get(&vec, 1);
    TEST_ASSERT(*out_value == 3, "value at index 1 should be 3");

    idcu_vector_destroy(&vec);
    printf("PASS: test_vector_insert_remove\n");
    return 0;
}

static int test_vector_clear(void) {
    idcu_Vector vec;
    idcu_vector_init(&vec, sizeof(int), 4);

    int value = 42;
    idcu_vector_push_back(&vec, &value);
    idcu_vector_push_back(&vec, &value);

    size_t size = idcu_vector_size(&vec);
    TEST_ASSERT(size == 2, "size should be 2");

    idcu_vector_clear(&vec);

    size = idcu_vector_size(&vec);
    TEST_ASSERT(size == 0, "size should be 0 after clear");

    idcu_vector_destroy(&vec);
    printf("PASS: test_vector_clear\n");
    return 0;
}

int main(void) {
    int failures = 0;

    if (test_vector_init_destroy() != 0)
        failures++;
    if (test_vector_push_back() != 0)
        failures++;
    if (test_vector_get_set() != 0)
        failures++;
    if (test_vector_pop_back() != 0)
        failures++;
    if (test_vector_insert_remove() != 0)
        failures++;
    if (test_vector_clear() != 0)
        failures++;

    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
