# idcu-module-system API Documentation

Module system for dynamic module loading and management.

## Module Definition

```c
typedef struct idcu_ModuleDef {
    const char* name;
    const char* version_str;
    const char* description;
    const char* category;
    
    idcu_ErrorCode (*init)(void);
    idcu_ErrorCode (*start)(void);
    idcu_ErrorCode (*stop)(void);
    idcu_ErrorCode (*destroy)(void);
    
    const char** dependencies;
    size_t dependency_count;
} idcu_ModuleDef;
```

## Module System

```c
typedef struct idcu_ModuleSystem idcu_ModuleSystem;
```

### Module System Functions

```c
int idcu_module_system_init(idcu_ModuleSystem* system);
void idcu_module_system_destroy(idcu_ModuleSystem* system);

int idcu_module_system_register(idcu_ModuleSystem* system, const idcu_ModuleDef* module_def);
int idcu_module_system_unregister(idcu_ModuleSystem* system, const char* name);

int idcu_module_system_find_by_name(idcu_ModuleSystem* system, const char* name, idcu_Module** out_module);
int idcu_module_system_get_all(idcu_ModuleSystem* system, idcu_ModuleInfo** out_infos, size_t* out_count);
int idcu_module_system_get_by_category(idcu_ModuleSystem* system, const char* category, idcu_ModuleInfo** out_infos, size_t* out_count);

int idcu_module_system_init_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_start_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_stop_module(idcu_ModuleSystem* system, const char* name);
int idcu_module_system_destroy_module(idcu_ModuleSystem* system, const char* name);

int idcu_module_system_init_all(idcu_ModuleSystem* system);
int idcu_module_system_start_all(idcu_ModuleSystem* system);
int idcu_module_system_stop_all(idcu_ModuleSystem* system);
int idcu_module_system_destroy_all(idcu_ModuleSystem* system);
```

## Module Info

```c
typedef struct {
    const char* name;
    const char* version_str;
    const char* description;
    const char* category;
    idcu_ModuleState state;
} idcu_ModuleInfo;
```

## Module States

```c
typedef enum {
    IDCU_MODULE_STATE_UNREGISTERED,
    IDCU_MODULE_STATE_REGISTERED,
    IDCU_MODULE_STATE_INITIALIZED,
    IDCU_MODULE_STATE_STARTED,
    IDCU_MODULE_STATE_STOPPED,
    IDCU_MODULE_STATE_DESTROYED
} idcu_ModuleState;
```

## Example

```c
#include <idcu/module/module.h>
#include <stdio.h>

// Define a module
static idcu_ErrorCode my_module_init(void) {
    printf("My module initialized\n");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_start(void) {
    printf("My module started\n");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_stop(void) {
    printf("My module stopped\n");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_destroy(void) {
    printf("My module destroyed\n");
    return IDCU_ERR_OK;
}

static const idcu_ModuleDef my_module = {
    .name = "my-module",
    .version_str = "1.0.0",
    .description = "My example module",
    .category = "example",
    .init = my_module_init,
    .start = my_module_start,
    .stop = my_module_stop,
    .destroy = my_module_destroy,
    .dependencies = NULL,
    .dependency_count = 0
};

int main(void) {
    idcu_ModuleSystem system;
    idcu_module_system_init(&system);
    
    // Register the module
    idcu_module_system_register(&system, &my_module);
    
    // Initialize and start
    idcu_module_system_init_module(&system, "my-module");
    idcu_module_system_start_module(&system, "my-module");
    
    // Stop and destroy
    idcu_module_system_stop_module(&system, "my-module");
    idcu_module_system_destroy_module(&system, "my-module");
    
    idcu_module_system_destroy(&system);
    return 0;
}
```
