#include <idcu/module/module.h>
#include <idcu/log/log.h>
#include <stdio.h>
#include <stdlib.h>

// Module 1: Hello Module
static int hello_module_init(void) {
    printf("Hello Module: Initializing...\n");
    return IDCU_ERR_OK;
}

static int hello_module_start(void) {
    printf("Hello Module: Starting...\n");
    printf("Hello Module: Hello, World!\n");
    return IDCU_ERR_OK;
}

static int hello_module_stop(void) {
    printf("Hello Module: Stopping...\n");
    return IDCU_ERR_OK;
}

static int hello_module_destroy(void) {
    printf("Hello Module: Destroying...\n");
    return IDCU_ERR_OK;
}

static idcu_ModuleDef hello_module_def = {
    .name = "hello_module",
    .version_str = "1.0.0",
    .description = "A simple hello world module",
    .category = "demo",
    .init = hello_module_init,
    .start = hello_module_start,
    .stop = hello_module_stop,
    .destroy = hello_module_destroy,
    .dependencies = NULL,
    .dependency_count = 0
};

// Module 2: Counter Module
static int counter = 0;

static int counter_module_init(void) {
    printf("Counter Module: Initializing...\n");
    counter = 0;
    return IDCU_ERR_OK;
}

static int counter_module_start(void) {
    printf("Counter Module: Starting...\n");
    return IDCU_ERR_OK;
}

static int counter_module_stop(void) {
    printf("Counter Module: Stopping...\n");
    return IDCU_ERR_OK;
}

static int counter_module_destroy(void) {
    printf("Counter Module: Destroying...\n");
    return IDCU_ERR_OK;
}

static idcu_ModuleDependency counter_deps[] = {
    {"hello_module", "1.0.0"}
};

static idcu_ModuleDef counter_module_def = {
    .name = "counter_module",
    .version_str = "1.0.0",
    .description = "A simple counter module",
    .category = "demo",
    .init = counter_module_init,
    .start = counter_module_start,
    .stop = counter_module_stop,
    .destroy = counter_module_destroy,
    .dependencies = counter_deps,
    .dependency_count = 1
};

static void print_module_info(idcu_ModuleSystem* system) {
    idcu_ModuleInfo* infos;
    size_t count;
    
    int ret = idcu_module_system_get_all(system, &infos, &count);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to get modules: %s\n", idcu_error_message(ret));
        return;
    }
    
    printf("\nRegistered Modules (%zu):\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("  - %s (v%s) [ID: %u] - State: %s\n",
               infos[i].name,
               infos[i].version,
               infos[i].id,
               idcu_module_state_to_str(infos[i].state));
    }
    
    free(infos);
}

int main(void) {
    printf("=== IDCU Module System Example ===\n\n");
    
    idcu_ModuleSystem system;
    int ret;
    
    // Initialize module system
    printf("Step 1: Initializing module system...\n");
    ret = idcu_module_system_init(&system);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init module system: %s\n", idcu_error_message(ret));
        return 1;
    }
    printf("Module system initialized successfully.\n\n");
    
    // Register modules
    printf("Step 2: Registering modules...\n");
    ret = idcu_module_system_register(&system, &hello_module_def);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to register hello_module: %s\n", idcu_error_message(ret));
        idcu_module_system_destroy(&system);
        return 1;
    }
    printf("Registered: hello_module\n");
    
    ret = idcu_module_system_register(&system, &counter_module_def);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to register counter_module: %s\n", idcu_error_message(ret));
        idcu_module_system_destroy(&system);
        return 1;
    }
    printf("Registered: counter_module\n\n");
    
    print_module_info(&system);
    
    // Initialize all modules
    printf("\nStep 3: Initializing all modules...\n");
    ret = idcu_module_system_init_all(&system);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init modules: %s\n", idcu_error_message(ret));
        idcu_module_system_destroy(&system);
        return 1;
    }
    print_module_info(&system);
    
    // Start all modules
    printf("\nStep 4: Starting all modules...\n");
    ret = idcu_module_system_start_all(&system);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to start modules: %s\n", idcu_error_message(ret));
        idcu_module_system_destroy(&system);
        return 1;
    }
    print_module_info(&system);
    
    // Stop all modules
    printf("\nStep 5: Stopping all modules...\n");
    ret = idcu_module_system_stop_all(&system);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to stop modules: %s\n", idcu_error_message(ret));
        idcu_module_system_destroy(&system);
        return 1;
    }
    print_module_info(&system);
    
    // Destroy all modules
    printf("\nStep 6: Destroying all modules...\n");
    ret = idcu_module_system_destroy_all(&system);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to destroy modules: %s\n", idcu_error_message(ret));
        idcu_module_system_destroy(&system);
        return 1;
    }
    print_module_info(&system);
    
    // Cleanup
    printf("\nStep 7: Destroying module system...\n");
    idcu_module_system_destroy(&system);
    
    printf("\n=== Module System Example Complete ===\n");
    return 0;
}
