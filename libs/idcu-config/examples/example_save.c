#include "idcu/config/config.h"
#include <stdio.h>

int main(void) {
    printf("idcu-config save example\n");
    printf("=======================\n\n");

    int ret = idcu_config_init(NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to initialize config: %d\n", ret);
        return 1;
    }

    // Set some configuration values
    idcu_config_set_string("general", "log_level", "info");
    idcu_config_set_int("network", "port", 8080);
    idcu_config_set_bool("network", "enabled", 1);
    idcu_config_set_string("database", "host", "localhost");

    // Save to file
    const char *filename = "example_config.ini";
    ret = idcu_config_save(filename);
    if (ret == IDCU_ERR_SUCCESS) {
        printf("Configuration saved to %s\n", filename);
    } else {
        printf("Failed to save config: %d\n", ret);
        idcu_config_shutdown();
        return 1;
    }

    idcu_config_shutdown();

    // Now reload it
    printf("\nReloading configuration...\n");
    ret = idcu_config_init(filename);
    if (ret == IDCU_ERR_SUCCESS) {
        printf("Configuration reloaded successfully!\n");
        printf("log_level: %s\n", idcu_config_get_string("general", "log_level", "unknown"));
        printf("port: %d\n", idcu_config_get_int("network", "port", 0));
    } else {
        printf("Failed to reload config: %d\n", ret);
    }

    idcu_config_shutdown();
    printf("\nExample completed!\n");
    return 0;
}
