#include "idcu/config/config.h"
#include <stdio.h>

int main(void) {
    printf("idcu-config basic example\n");
    printf("=========================\n\n");
    
    int ret = idcu_config_init(NULL);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("Failed to initialize config: %d\n", ret);
        return 1;
    }
    
    // Set some configuration values
    idcu_config_set_string("app", "name", "MyApp");
    idcu_config_set_int("app", "version", 1);
    idcu_config_set_bool("app", "debug", 1);
    idcu_config_set_double("app", "timeout", 30.5);
    
    // Read and print them
    printf("[app]\n");
    printf("name: %s\n", idcu_config_get_string("app", "name", "unknown"));
    printf("version: %d\n", idcu_config_get_int("app", "version", 0));
    printf("debug: %s\n", idcu_config_get_bool("app", "debug", 0) ? "true" : "false");
    printf("timeout: %.1f\n", idcu_config_get_double("app", "timeout", 0.0));
    
    idcu_config_shutdown();
    printf("\nExample completed successfully!\n");
    return 0;
}
