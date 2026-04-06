#include "idcu/common/hash_map.h"
#include "idcu/common/error_code.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        return 1; \
    } \
} while(0)

static int test_hash_map_init_destroy(void) {
    idcu_HashMap map;
    int ret = idcu_hash_map_init(&map, sizeof(int), 8);
    TEST_ASSERT(ret == IDCU_ERR_OK, "hash_map_init should succeed");
    
    idcu_hash_map_destroy(&map);
    printf("PASS: test_hash_map_init_destroy\n");
    return 0;
}

static int test_hash_map_insert_get(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, sizeof(int), 8);
    
    int value = 42;
    int ret = idcu_hash_map_insert(&map, "key1", &value);
    TEST_ASSERT(ret == IDCU_ERR_OK, "insert should succeed");
    
    int out_value;
    ret = idcu_hash_map_get(&map, "key1", &out_value);
    TEST_ASSERT(ret == IDCU_ERR_OK, "get should succeed");
    TEST_ASSERT(out_value == 42, "value should be 42");
    
    idcu_hash_map_destroy(&map);
    printf("PASS: test_hash_map_insert_get\n");
    return 0;
}

static int test_hash_map_contains(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, sizeof(int), 8);
    
    int value = 42;
    idcu_hash_map_insert(&map, "key1", &value);
    
    bool contains = idcu_hash_map_contains(&map, "key1");
    TEST_ASSERT(contains == true, "should contain key1");
    
    contains = idcu_hash_map_contains(&map, "nonexistent");
    TEST_ASSERT(contains == false, "should not contain nonexistent");
    
    idcu_hash_map_destroy(&map);
    printf("PASS: test_hash_map_contains\n");
    return 0;
}

static int test_hash_map_remove(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, sizeof(int), 8);
    
    int value = 42;
    idcu_hash_map_insert(&map, "key1", &value);
    
    int ret = idcu_hash_map_remove(&map, "key1");
    TEST_ASSERT(ret == IDCU_ERR_OK, "remove should succeed");
    
    bool contains = idcu_hash_map_contains(&map, "key1");
    TEST_ASSERT(contains == false, "key1 should be removed");
    
    idcu_hash_map_destroy(&map);
    printf("PASS: test_hash_map_remove\n");
    return 0;
}

static int test_hash_map_clear(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, sizeof(int), 8);
    
    int value = 42;
    idcu_hash_map_insert(&map, "key1", &value);
    idcu_hash_map_insert(&map, "key2", &value);
    
    size_t size = idcu_hash_map_size(&map);
    TEST_ASSERT(size == 2, "size should be 2");
    
    idcu_hash_map_clear(&map);
    
    size = idcu_hash_map_size(&map);
    TEST_ASSERT(size == 0, "size should be 0 after clear");
    
    idcu_hash_map_destroy(&map);
    printf("PASS: test_hash_map_clear\n");
    return 0;
}

static int test_hash_map_multiple_inserts(void) {
    idcu_HashMap map;
    idcu_hash_map_init(&map, sizeof(int), 8);
    
    for (int i = 0; i < 100; i++) {
        char key[32];
        sprintf(key, "key%d", i);
        int value = i * 2;
        int ret = idcu_hash_map_insert(&map, key, &value);
        TEST_ASSERT(ret == IDCU_ERR_OK, "insert should succeed");
    }
    
    size_t size = idcu_hash_map_size(&map);
    TEST_ASSERT(size == 100, "size should be 100");
    
    for (int i = 0; i < 100; i++) {
        char key[32];
        sprintf(key, "key%d", i);
        int out_value;
        int ret = idcu_hash_map_get(&map, key, &out_value);
        TEST_ASSERT(ret == IDCU_ERR_OK, "get should succeed");
        TEST_ASSERT(out_value == i * 2, "value should match");
    }
    
    idcu_hash_map_destroy(&map);
    printf("PASS: test_hash_map_multiple_inserts\n");
    return 0;
}

int main(void) {
    int failures = 0;
    
    if (test_hash_map_init_destroy() != 0) failures++;
    if (test_hash_map_insert_get() != 0) failures++;
    if (test_hash_map_contains() != 0) failures++;
    if (test_hash_map_remove() != 0) failures++;
    if (test_hash_map_clear() != 0) failures++;
    if (test_hash_map_multiple_inserts() != 0) failures++;
    
    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
