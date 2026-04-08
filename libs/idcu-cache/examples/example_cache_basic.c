#include <idcu/cache/cache.h>
#include <idcu/common/error_code.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== idcu-cache Basic Example ===\n\n");

    int ret;
    idcu_Cache *cache = NULL;

    printf("1. Initialize Cache (LRU Policy)\n");
    printf("-----------------------------------\n");
    ret = idcu_cache_init(&cache, 100, IDCU_CACHE_POLICY_LRU);
    if (ret != IDCU_ERR_OK) {
        printf("Cache init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("Cache initialized successfully\n\n");

    printf("2. Put and Get String Values\n");
    printf("------------------------------\n");
    ret = idcu_cache_put_string(cache, "key1", "Hello, World!", 0);
    if (ret != IDCU_ERR_OK) {
        printf("Put string failed: %s\n", idcu_err_to_str(ret));
    } else {
        printf("Put string: key1 = \"Hello, World!\"\n");
    }

    char buffer[256];
    ret = idcu_cache_get_string(cache, "key1", buffer, sizeof(buffer));
    if (ret == IDCU_ERR_OK) {
        printf("Get string: key1 = \"%s\"\n", buffer);
    } else {
        printf("Get string failed: %s\n", idcu_err_to_str(ret));
    }
    printf("\n");

    printf("3. Put and Get Integer Values\n");
    printf("-------------------------------\n");
    ret = idcu_cache_put_int(cache, "counter", 42, 0);
    if (ret == IDCU_ERR_OK) {
        printf("Put int: counter = 42\n");
    }

    int64_t int_value;
    ret = idcu_cache_get_int(cache, "counter", &int_value);
    if (ret == IDCU_ERR_OK) {
        printf("Get int: counter = %lld\n", (long long)int_value);
    }
    printf("\n");

    printf("4. Check Cache Contains Key\n");
    printf("-----------------------------\n");
    if (idcu_cache_contains(cache, "key1")) {
        printf("Cache contains key: key1\n");
    }
    if (!idcu_cache_contains(cache, "nonexistent")) {
        printf("Cache does NOT contain key: nonexistent\n");
    }
    printf("\n");

    printf("5. Cache Statistics\n");
    printf("--------------------\n");
    printf("Cache size: %zu bytes\n", idcu_cache_size(cache));
    printf("Cache entry count: %zu\n", idcu_cache_count(cache));
    printf("\n");

    printf("6. Remove Key from Cache\n");
    printf("--------------------------\n");
    ret = idcu_cache_remove(cache, "key1");
    if (ret == IDCU_ERR_OK) {
        printf("Removed key: key1\n");
    }
    if (!idcu_cache_contains(cache, "key1")) {
        printf("Key1 is no longer in cache\n");
    }
    printf("\n");

    printf("7. Clear All Cache Entries\n");
    printf("----------------------------\n");
    ret = idcu_cache_clear(cache);
    if (ret == IDCU_ERR_OK) {
        printf("Cache cleared\n");
    }
    printf("Entry count after clear: %zu\n", idcu_cache_count(cache));
    printf("\n");

    printf("8. Test with TTL (Time To Live)\n");
    printf("----------------------------------\n");
    ret = idcu_cache_put_string(cache, "temp_key", "This expires soon", 1000);
    if (ret == IDCU_ERR_OK) {
        printf("Put temp_key with 1 second TTL\n");
    }

    if (idcu_cache_contains(cache, "temp_key")) {
        printf("temp_key is present immediately after put\n");
    }
    printf("\n");

    printf("9. Destroy Cache\n");
    printf("------------------\n");
    idcu_cache_destroy(cache);
    printf("Cache destroyed\n\n");

    printf("=== Example Complete ===\n");
    return 0;
}
