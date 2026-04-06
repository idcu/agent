#include "idcu/cache/cache.h"
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

static int test_cache_init_destroy(void) {
    idcu_Cache* cache;
    int ret = idcu_cache_init(&cache, 10, IDCU_CACHE_POLICY_LRU);
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_cache_init should succeed");
    
    idcu_cache_destroy(cache);
    printf("PASS: test_cache_init_destroy\n");
    return 0;
}

static int test_cache_put_get_string(void) {
    idcu_Cache* cache;
    idcu_cache_init(&cache, 10, IDCU_CACHE_POLICY_LRU);
    
    int ret = idcu_cache_put_string(cache, "test_key", "test_value", 0);
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "put_string should succeed");
    
    char buffer[64];
    ret = idcu_cache_get_string(cache, "test_key", buffer, sizeof(buffer));
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "get_string should succeed");
    TEST_ASSERT(strcmp(buffer, "test_value") == 0, "value should match");
    
    idcu_cache_destroy(cache);
    printf("PASS: test_cache_put_get_string\n");
    return 0;
}

static int test_cache_put_get_int(void) {
    idcu_Cache* cache;
    idcu_cache_init(&cache, 10, IDCU_CACHE_POLICY_LRU);
    
    int ret = idcu_cache_put_int(cache, "test_key", 42, 0);
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "put_int should succeed");
    
    int64_t value;
    ret = idcu_cache_get_int(cache, "test_key", &value);
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "get_int should succeed");
    TEST_ASSERT(value == 42, "value should be 42");
    
    idcu_cache_destroy(cache);
    printf("PASS: test_cache_put_get_int\n");
    return 0;
}

static int test_cache_contains(void) {
    idcu_Cache* cache;
    idcu_cache_init(&cache, 10, IDCU_CACHE_POLICY_LRU);
    
    idcu_cache_put_string(cache, "test_key", "test_value", 0);
    
    int ret = idcu_cache_contains(cache, "test_key");
    TEST_ASSERT(ret != 0, "contains should return true for existing key");
    
    ret = idcu_cache_contains(cache, "nonexistent");
    TEST_ASSERT(ret == 0, "contains should return false for non-existent key");
    
    idcu_cache_destroy(cache);
    printf("PASS: test_cache_contains\n");
    return 0;
}

static int test_cache_remove(void) {
    idcu_Cache* cache;
    idcu_cache_init(&cache, 10, IDCU_CACHE_POLICY_LRU);
    
    idcu_cache_put_string(cache, "test_key", "test_value", 0);
    
    int ret = idcu_cache_remove(cache, "test_key");
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "remove should succeed");
    
    ret = idcu_cache_contains(cache, "test_key");
    TEST_ASSERT(ret == 0, "key should be removed");
    
    idcu_cache_destroy(cache);
    printf("PASS: test_cache_remove\n");
    return 0;
}

static int test_cache_clear(void) {
    idcu_Cache* cache;
    idcu_cache_init(&cache, 10, IDCU_CACHE_POLICY_LRU);
    
    idcu_cache_put_string(cache, "key1", "value1", 0);
    idcu_cache_put_string(cache, "key2", "value2", 0);
    
    TEST_ASSERT(idcu_cache_count(cache) == 2, "should have 2 entries");
    
    int ret = idcu_cache_clear(cache);
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "clear should succeed");
    
    TEST_ASSERT(idcu_cache_count(cache) == 0, "should have 0 entries after clear");
    
    idcu_cache_destroy(cache);
    printf("PASS: test_cache_clear\n");
    return 0;
}

static int test_cache_lru_eviction(void) {
    idcu_Cache* cache;
    idcu_cache_init(&cache, 3, IDCU_CACHE_POLICY_LRU);
    
    idcu_cache_put_string(cache, "key1", "value1", 0);
    idcu_cache_put_string(cache, "key2", "value2", 0);
    idcu_cache_put_string(cache, "key3", "value3", 0);
    
    TEST_ASSERT(idcu_cache_count(cache) == 3, "should have 3 entries");
    
    idcu_cache_put_string(cache, "key4", "value4", 0);
    
    TEST_ASSERT(idcu_cache_count(cache) == 3, "should still have 3 entries");
    TEST_ASSERT(idcu_cache_contains(cache, "key1") == 0, "key1 should be evicted");
    
    idcu_cache_destroy(cache);
    printf("PASS: test_cache_lru_eviction\n");
    return 0;
}

int main(void) {
    int failures = 0;
    
    if (test_cache_init_destroy() != 0) failures++;
    if (test_cache_put_get_string() != 0) failures++;
    if (test_cache_put_get_int() != 0) failures++;
    if (test_cache_contains() != 0) failures++;
    if (test_cache_remove() != 0) failures++;
    if (test_cache_clear() != 0) failures++;
    if (test_cache_lru_eviction() != 0) failures++;
    
    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
