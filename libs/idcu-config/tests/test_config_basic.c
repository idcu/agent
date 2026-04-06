#include "idcu/config/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("Testing idcu-config basic functionality...\n");
    
    int ret = idcu_config_init(NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to initialize config: %d\n", ret);
        return 1;
    }
    
    // Test setting and getting string
    idcu_config_set_string("test", "key1", "value1");
    const char* str_val = idcu_config_get_string("test", "key1", "default");
    if (strcmp(str_val, "value1") != 0) {
        printf("String test failed: expected 'value1', got '%s'\n", str_val);
        idcu_config_shutdown();
        return 1;
    }
    
    // Test setting and getting int
    idcu_config_set_int("test", "key2", 42);
    int int_val = idcu_config_get_int("test", "key2", 0);
    if (int_val != 42) {
        printf("Int test failed: expected 42, got %d\n", int_val);
        idcu_config_shutdown();
        return 1;
    }
    
    // Test setting and getting bool
    idcu_config_set_bool("test", "key3", 1);
    int bool_val = idcu_config_get_bool("test", "key3", 0);
    if (bool_val != 1) {
        printf("Bool test failed: expected 1, got %d\n", bool_val);
        idcu_config_shutdown();
        return 1;
    }
    
    // Test section and key existence
    if (!idcu_config_has_section("test")) {
        printf("Section test failed\n");
        idcu_config_shutdown();
        return 1;
    }
    
    if (!idcu_config_has_key("test", "key1")) {
        printf("Key test failed\n");
        idcu_config_shutdown();
        return 1;
    }
    
    // Test remove key
    idcu_config_remove_key("test", "key1");
    if (idcu_config_has_key("test", "key1")) {
        printf("Remove key test failed\n");
        idcu_config_shutdown();
        return 1;
    }
    
    // Test remove section
    idcu_config_remove_section("test");
    if (idcu_config_has_section("test")) {
        printf("Remove section test failed\n");
        idcu_config_shutdown();
        return 1;
    }
    
    idcu_config_shutdown();
    printf("All tests passed!\n");
    return 0;
}
