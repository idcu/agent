/**
 * @file hotload_module_example.c
 * @brief 热加载模块示例
 *
 * 本示例演示如何使用IDCU的动态模块加载功能，
 * 实现模块的热加载、卸载和重新加载。
 */

#include "module_def.h"
#include "dynamic_module.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#define MODULES_DIR "./modules"

static void print_loaded_modules(idcu_DynamicLoader* loader) {
    uint32_t count = idcu_dynamic_loader_get_count(loader);
    printf("\n=== Loaded Modules (%u) ===\n", count);
    
    for (uint32_t i = 0; i < count; i++) {
        idcu_DynamicModule* mod = idcu_dynamic_loader_get_at(loader, i);
        if (mod) {
            const char* state_str;
            switch (mod->state) {
                case IDCU_MODULE_STATE_UNLOADED: state_str = "UNLOADED"; break;
                case IDCU_MODULE_STATE_LOADED: state_str = "LOADED"; break;
                case IDCU_MODULE_STATE_INITIALIZED: state_str = "INITIALIZED"; break;
                case IDCU_MODULE_STATE_RUNNING: state_str = "RUNNING"; break;
                case IDCU_MODULE_STATE_STOPPED: state_str = "STOPPED"; break;
                default: state_str = "UNKNOWN"; break;
            }
            printf("  [%u] %s - %s (ref: %u)\n", i, mod->name, state_str, mod->ref_count);
        }
    }
    printf("============================\n\n");
}

static int demonstrate_hotload(void) {
    printf("====================================\n");
    printf("Hot Load Module Example\n");
    printf("====================================\n\n");
    
    idcu_DynamicLoader loader;
    int ret = idcu_dynamic_loader_init(&loader, MODULES_DIR);
    if (ret != IDCU_ERR_OK) {
        fprintf(stderr, "Failed to initialize dynamic loader: %d\n", ret);
        return 1;
    }
    
    printf("Dynamic loader initialized, module directory: %s\n\n", MODULES_DIR);
    
    printf("Step 1: Load a dynamic module...\n");
    ret = idcu_dynamic_loader_load_module(&loader, "sample_module", "./sample_module.so");
    if (ret == IDCU_ERR_OK) {
        printf("Module loaded successfully!\n");
    } else {
        printf("Note: Sample module not found (this is expected in demo mode)\n");
        printf("      In real usage, compile modules to shared libraries first.\n");
    }
    
    print_loaded_modules(&loader);
    
    printf("Step 2: Try to load another module (demo)...\n");
    ret = idcu_dynamic_loader_load_module(&loader, "another_module", "./another_module.dll");
    if (ret == IDCU_ERR_OK) {
        printf("Module loaded successfully!\n");
    } else {
        printf("Note: Second sample module not found (demo mode)\n");
    }
    
    print_loaded_modules(&loader);
    
    printf("Step 3: Find and initialize a module (demo)...\n");
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "sample_module");
    if (mod) {
        printf("Found module: %s\n", mod->name);
        ret = idcu_dynamic_module_init(mod);
        if (ret == IDCU_ERR_OK) {
            printf("Module initialized!\n");
        }
    } else {
        printf("Module not found (demo mode)\n");
    }
    
    print_loaded_modules(&loader);
    
    printf("Step 4: Unload a module (demo)...\n");
    ret = idcu_dynamic_loader_unload_module(&loader, "sample_module");
    if (ret == IDCU_ERR_OK) {
        printf("Module unloaded successfully!\n");
    } else {
        printf("Note: Module not loaded or unload failed (demo mode)\n");
    }
    
    print_loaded_modules(&loader);
    
    printf("Step 5: Hot reload a module (demo)...\n");
    ret = idcu_dynamic_module_reload(&loader, "sample_module", "./sample_module_v2.so");
    if (ret == IDCU_ERR_OK) {
        printf("Module reloaded successfully!\n");
    } else {
        printf("Note: Hot reload demo - would load new version of module\n");
    }
    
    print_loaded_modules(&loader);
    
    printf("Step 6: Hotplug operations (demo)...\n");
    printf("  Hotplug load: Would detect and load new module at runtime\n");
    printf("  Hotplug unload: Would safely unload module at runtime\n\n");
    
    idcu_dynamic_loader_destroy(&loader);
    
    printf("Dynamic loader cleaned up\n\n");
    
    return 0;
}

int main(void) {
    printf("====================================\n");
    printf("Hot Load Module Example\n");
    printf("====================================\n\n");
    
    printf("This example demonstrates the dynamic module loading system.\n\n");
    printf("Key features demonstrated:\n");
    printf("  - Dynamic module loading/unloading\n");
    printf("  - Hot reload of modules\n");
    printf("  - Hotplug support (runtime module detection)\n");
    printf("  - Module state management\n");
    printf("  - Reference counting\n\n");
    printf("In real usage:\n");
    printf("  1. Compile your module as a shared library (.so on Linux, .dll on Windows)\n");
    printf("  2. Place the library in the modules directory\n");
    printf("  3. Use the dynamic loader API to load/unload at runtime\n\n");
    
    return demonstrate_hotload();
}
