#include <idcu/testframework/testframework.h>
#include <idcu/cache/cache.h>
#include <idcu/cache/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IDCU_TEST_CASE(cache, init_destroy) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    int ret = idcu_cache_init(&cache, &config);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, put_get_string) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    int ret = idcu_cache_put_string(&cache, "key1", "value1");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    char buffer[64];
    ret = idcu_cache_get_string(&cache, "key1", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_STRING_EQUAL("value1", buffer);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, put_get_int) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    int ret = idcu_cache_put_int(&cache, "key1", 42);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    int64_t value;
    ret = idcu_cache_get_int(&cache, "key1", &value);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(42, value);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, contains) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    IDCU_TEST_ASSERT(!idcu_cache_contains(&cache, "key1"));
    
    idcu_cache_put_string(&cache, "key1", "value1");
    IDCU_TEST_ASSERT(idcu_cache_contains(&cache, "key1"));
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, remove) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    idcu_cache_put_string(&cache, "key1", "value1");
    
    int ret = idcu_cache_remove(&cache, "key1");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT(!idcu_cache_contains(&cache, "key1"));
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, clear) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    idcu_cache_put_string(&cache, "key1", "value1");
    idcu_cache_put_string(&cache, "key2", "value2");
    
    IDCU_TEST_ASSERT_EQUAL(2, idcu_cache_get_count(&cache));
    
    int ret = idcu_cache_clear(&cache);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    IDCU_TEST_ASSERT_EQUAL(0, idcu_cache_get_count(&cache));
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, get_count) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    IDCU_TEST_ASSERT_EQUAL(0, idcu_cache_get_count(&cache));
    
    idcu_cache_put_string(&cache, "key1", "value1");
    idcu_cache_put_string(&cache, "key2", "value2");
    
    IDCU_TEST_ASSERT_EQUAL(2, idcu_cache_get_count(&cache));
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, hit_miss_stats) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    IDCU_TEST_ASSERT_EQUAL(0, idcu_cache_get_hit_count(&cache));
    IDCU_TEST_ASSERT_EQUAL(0, idcu_cache_get_miss_count(&cache));
    
    idcu_cache_put_string(&cache, "key1", "value1");
    
    char buffer[64];
    idcu_cache_get_string(&cache, "key1", buffer, sizeof(buffer));
    
    IDCU_TEST_ASSERT(idcu_cache_get_hit_count(&cache) >= 1);
    
    idcu_cache_get_string(&cache, "nonexistent", buffer, sizeof(buffer));
    
    IDCU_TEST_ASSERT(idcu_cache_get_miss_count(&cache) >= 1);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, eviction) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 3;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    idcu_cache_put_string(&cache, "key1", "value1");
    idcu_cache_put_string(&cache, "key2", "value2");
    idcu_cache_put_string(&cache, "key3", "value3");
    idcu_cache_put_string(&cache, "key4", "value4");
    
    IDCU_TEST_ASSERT_EQUAL(3, idcu_cache_get_count(&cache));
    IDCU_TEST_ASSERT(idcu_cache_get_evict_count(&cache) >= 1);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, overwrite) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    idcu_cache_put_string(&cache, "key1", "value1");
    
    char buffer[64];
    idcu_cache_get_string(&cache, "key1", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_STRING_EQUAL("value1", buffer);
    
    idcu_cache_put_string(&cache, "key1", "new_value");
    idcu_cache_get_string(&cache, "key1", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_STRING_EQUAL("new_value", buffer);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, get_nonexistent) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    char buffer[64];
    int ret = idcu_cache_get_string(&cache, "nonexistent", buffer, sizeof(buffer));
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, memory_usage) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    IDCU_TEST_ASSERT_EQUAL(0, idcu_cache_get_memory_usage(&cache));
    
    idcu_cache_put_string(&cache, "key1", "value1");
    size_t usage = idcu_cache_get_memory_usage(&cache);
    IDCU_TEST_ASSERT(usage > 0);
    
    idcu_cache_destroy(&cache);
}

IDCU_TEST_CASE(cache, hit_rate) {
    idcu_Cache cache;
    idcu_CacheConfig config = {0};
    config.max_entries = 100;
    config.max_memory = 1024 * 1024;
    config.policy = IDCU_CACHE_POLICY_LRU;
    
    idcu_cache_init(&cache, &config);
    
    IDCU_TEST_ASSERT_DOUBLE_EQUAL(0.0, idcu_cache_get_hit_rate(&cache), 0.001);
    
    idcu_cache_put_string(&cache, "key1", "value1");
    
    char buffer[64];
    idcu_cache_get_string(&cache, "key1", buffer, sizeof(buffer));
    
    double hit_rate = idcu_cache_get_hit_rate(&cache);
    IDCU_TEST_ASSERT(hit_rate > 0.0);
    
    idcu_cache_destroy(&cache);
}

int main(void) {
    return idcu_test_run_all();
}
