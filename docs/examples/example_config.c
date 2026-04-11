#include <idcu/config/config.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== IDCU Config Example ===\n\n");
    
    // Example 1: Create and save config
    printf("Example 1: Create and save config\n");
    int ret = idcu_config_init("example_config.ini");
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init config: %s\n", idcu_error_message(ret));
        return 1;
    }
    
    idcu_config_set_string("app", "name", "My Application");
    idcu_config_set_int("app", "version", 1);
    idcu_config_set_bool("app", "debug", 1);
    
    idcu_config_set_string("database", "host", "localhost");
    idcu_config_set_int("database", "port", 5432);
    idcu_config_set_string("database", "user", "admin");
    
    ret = idcu_config_save("example_config.ini");
    if (ret != IDCU_ERR_OK) {
        printf("Failed to save config: %s\n", idcu_error_message(ret));
    } else {
        printf("Config saved to example_config.ini\n");
    }
    
    idcu_config_shutdown();
    printf("\n");
    
    // Example 2: Load and read config
    printf("Example 2: Load and read config\n");
    ret = idcu_config_init("example_config.ini");
    if (ret != IDCU_ERR_OK) {
        printf("Failed to load config: %s\n", idcu_error_message(ret));
        return 1;
    }
    
    const char* app_name = idcu_config_get_string("app", "name", "Unknown");
    int app_version = idcu_config_get_int("app", "version", 0);
    int debug = idcu_config_get_bool("app", "debug", 0);
    
    printf("App Name: %s\n", app_name);
    printf("App Version: %d\n", app_version);
    printf("Debug Mode: %s\n", debug ? "ON" : "OFF");
    
    const char* db_host = idcu_config_get_string("database", "host", "localhost");
    int db_port = idcu_config_get_int("database", "port", 5432);
    
    printf("Database: %s:%d\n", db_host, db_port);
    
    idcu_config_shutdown();
    printf("\n");
    
    // Example 3: Working with lists
    printf("Example 3: Working with lists\n");
    ret = idcu_config_init("example_config.ini");
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init config: %s\n", idcu_error_message(ret));
        return 1;
    }
    
    idcu_ConfigList list;
    idcu_config_get_list("cluster", "servers", ",", &list);
    
    printf("Cluster servers (%u):\n", list.count);
    for (size_t i = 0; i < list.count; i++) {
        printf("  - %s\n", list.items[i]);
    }
    
    idcu_config_shutdown();
    
    printf("\n=== Config Example Complete ===\n");
    return 0;
}
