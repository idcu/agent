#include <idcu/common/error_code.h>
#include <idcu/plugin/plugin.h>
#include <stdio.h>

int main(void) {
    printf("=== idcu-plugin Basic Example ===\n\n");

    int ret;

    printf("1. Initialize Plugin System\n");
    printf("----------------------------\n");
    ret = idcu_plugin_system_init();
    if (ret != IDCU_ERR_OK) {
        printf("Plugin system init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("Plugin system initialized\n\n");

    printf("2. Get Plugin Count\n");
    printf("---------------------\n");
    int plugin_count = idcu_plugin_get_count();
    printf("Currently loaded plugins: %d\n", plugin_count);
    printf("\n");

    printf("3. Scan Directory for Plugins\n");
    printf("--------------------------------\n");
    ret = idcu_plugin_scan_directory("./plugins");
    if (ret == IDCU_ERR_OK) {
        printf("Scanned directory: ./plugins\n");
    } else {
        printf("Scan directory failed (expected if no plugins): %s\n", idcu_err_to_str(ret));
    }
    printf("\n");

    printf("4. Try to Load All Plugins\n");
    printf("----------------------------\n");
    ret = idcu_plugin_load_all();
    if (ret == IDCU_ERR_OK) {
        printf("Load all plugins attempted\n");
    } else {
        printf("Load all failed (expected if no plugins): %s\n", idcu_err_to_str(ret));
    }
    printf("\n");

    printf("5. Check Plugin Count Again\n");
    printf("-----------------------------\n");
    plugin_count = idcu_plugin_get_count();
    printf("Loaded plugins after scan/load: %d\n", plugin_count);
    printf("\n");

    printf("6. Unload All Plugins\n");
    printf("-----------------------\n");
    ret = idcu_plugin_unload_all();
    if (ret == IDCU_ERR_OK) {
        printf("All plugins unloaded\n");
    }
    printf("\n");

    printf("7. Shutdown Plugin System\n");
    printf("---------------------------\n");
    idcu_plugin_system_shutdown();
    printf("Plugin system shutdown\n\n");

    printf("=== Example Complete ===\n");
    printf("\n");
    printf("Note: This example demonstrates the plugin API. To test with actual plugins,\n");
    printf(
        "you need to create plugin shared libraries and place them in the ./plugins directory.\n");
    return 0;
}
