#include <stdio.h>
#include <idcu/common/error_code.h>
#include <idcu/log/log.h>
#include <idcu/cache/cache.h>
#include <idcu/config/config.h>

void demonstrate_error_codes(void) {
    printf("=== Error Code Demonstration ===\n");
    printf("IDCU_ERR_OK: %d - %s\n", IDCU_ERR_OK, idcu_err_to_str(IDCU_ERR_OK));
    printf("IDCU_ERR_INVALID_ARG: %d - %s\n", IDCU_ERR_INVALID_ARG, idcu_err_to_str(IDCU_ERR_INVALID_ARG));
    printf("IDCU_ERR_OUT_OF_MEMORY: %d - %s\n", IDCU_ERR_OUT_OF_MEMORY, idcu_err_to_str(IDCU_ERR_OUT_OF_MEMORY));
    printf("IDCU_ERR_NOT_FOUND: %d - %s\n", IDCU_ERR_NOT_FOUND, idcu_err_to_str(IDCU_ERR_NOT_FOUND));
    printf("IDCU_ERR_ALREADY_EXISTS: %d - %s\n", IDCU_ERR_ALREADY_EXISTS, idcu_err_to_str(IDCU_ERR_ALREADY_EXISTS));
    printf("IDCU_ERR_TIMEOUT: %d - %s\n", IDCU_ERR_TIMEOUT, idcu_err_to_str(IDCU_ERR_TIMEOUT));
    printf("IDCU_ERR_PERMISSION_DENIED: %d - %s\n", IDCU_ERR_PERMISSION_DENIED, idcu_err_to_str(IDCU_ERR_PERMISSION_DENIED));
    printf("\n");
}

void demonstrate_cache_error_handling(void) {
    printf("=== Cache Error Handling ===\n");
    
    int ret;
    idcu_Cache* cache = NULL;

    printf("1. Try to get without init\n");
    char buffer[256];
    ret = idcu_cache_get_string(cache, "key", buffer, sizeof(buffer));
    printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
    printf("\n");

    printf("2. Initialize cache with invalid max_entries (0)\n");
    ret = idcu_cache_init(&cache, 0, IDCU_CACHE_POLICY_LRU);
    printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
    printf("\n");

    printf("3. Initialize properly\n");
    ret = idcu_cache_init(&cache, 10, IDCU_CACHE_POLICY_LRU);
    if (ret == IDCU_ERR_OK) {
        printf("   Init successful\n");
        
        printf("4. Get non-existent key\n");
        ret = idcu_cache_get_string(cache, "nonexistent", buffer, sizeof(buffer));
        printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
        printf("\n");

        printf("5. Put NULL key\n");
        ret = idcu_cache_put_string(cache, NULL, "value", 0);
        printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
        printf("\n");

        idcu_cache_destroy(cache);
        printf("Cache destroyed\n");
    }
    printf("\n");
}

void demonstrate_log_error_handling(void) {
    printf("=== Log Error Handling ===\n");
    
    int ret;

    printf("1. Try to log without init\n");
    ret = idcu_log_info("This should not work");
    printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
    printf("\n");

    printf("2. Initialize log\n");
    ret = idcu_log_init(IDCU_LOG_LEVEL_INFO, NULL);
    if (ret == IDCU_ERR_OK) {
        printf("   Init successful\n");
        
        printf("3. Log with invalid level\n");
        ret = idcu_log_set_level(999);
        printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
        printf("\n");

        idcu_log_shutdown();
        printf("Log shutdown\n");
    }
    printf("\n");
}

void demonstrate_config_error_handling(void) {
    printf("=== Config Error Handling ===\n");
    
    int ret;
    idcu_Config* config = NULL;

    printf("1. Try to load non-existent file\n");
    ret = idcu_config_load(&config, "nonexistent_config_file.cfg");
    printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
    printf("\n");

    printf("2. Initialize empty config\n");
    ret = idcu_config_init(&config);
    if (ret == IDCU_ERR_OK) {
        printf("   Init successful\n");
        
        printf("3. Get non-existent key\n");
        char value[256];
        ret = idcu_config_get_string(config, "nonexistent", value, sizeof(value));
        printf("   Result: %d - %s\n", ret, idcu_err_to_str(ret));
        printf("\n");

        idcu_config_destroy(config);
        printf("Config destroyed\n");
    }
    printf("\n");
}

int main(void) {
    printf("=== Error Handling Example ===\n\n");

    demonstrate_error_codes();
    demonstrate_cache_error_handling();
    demonstrate_log_error_handling();
    demonstrate_config_error_handling();

    printf("=== Error Handling Example Complete ===\n");
    return 0;
}
