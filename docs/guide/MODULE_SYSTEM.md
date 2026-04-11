# Module System Design

This document describes the design of the IDCU Agent module system.

## Overview

The module system provides a framework for dynamic component loading and lifecycle management.

## Core Concepts

### Module Definition

A module is defined by the `idcu_ModuleDef` structure:

```c
typedef struct idcu_ModuleDef {
    const char* name;           // Unique module identifier
    const char* version_str;    // Semantic version (e.g., "1.0.0")
    const char* description;    // Human-readable description
    const char* category;       // Module category
    
    // Lifecycle callbacks
    idcu_ErrorCode (*init)(void);
    idcu_ErrorCode (*start)(void);
    idcu_ErrorCode (*stop)(void);
    idcu_ErrorCode (*destroy)(void);
    
    // Dependencies
    const char** dependencies;
    size_t dependency_count;
} idcu_ModuleDef;
```

### Module Lifecycle

```
UNREGISTERED
    ↓ (register)
REGISTERED
    ↓ (init)
INITIALIZED
    ↓ (start)
STARTED ←──┐
    ↓ (stop)  │
STOPPED ─────┘
    ↓ (destroy)
DESTROYED
```

### State Transitions

| From State   | To State     | Trigger           |
|--------------|--------------|-------------------|
| UNREGISTERED | REGISTERED   | `register()`      |
| REGISTERED   | INITIALIZED  | `init_module()`   |
| INITIALIZED  | STARTED      | `start_module()`  |
| STARTED      | STOPPED      | `stop_module()`   |
| STOPPED      | STARTED      | `start_module()`  |
| STOPPED      | DESTROYED    | `destroy_module()`|

## Dependencies

### Dependency Declaration

Modules can declare dependencies on other modules:

```c
static const char* my_deps[] = {"module-a", "module-b"};

static const idcu_ModuleDef my_module = {
    .name = "my-module",
    .version_str = "1.0.0",
    .dependencies = my_deps,
    .dependency_count = 2,
    // ...
};
```

### Dependency Resolution

The module system ensures:
1. Dependencies are initialized before dependents
2. Dependencies are started before dependents
3. Dependents are stopped before dependencies
4. Dependents are destroyed before dependencies

### Circular Dependencies

Circular dependencies are detected and rejected during registration.

## Module System API

### Registration

```c
// Register a module
int idcu_module_system_register(idcu_ModuleSystem* system, 
                                 const idcu_ModuleDef* module_def);

// Unregister a module
int idcu_module_system_unregister(idcu_ModuleSystem* system, 
                                   const char* name);
```

### Lifecycle Management

```c
// Single module operations
int idcu_module_system_init_module(idcu_ModuleSystem* system, 
                                    const char* name);
int idcu_module_system_start_module(idcu_ModuleSystem* system, 
                                     const char* name);
int idcu_module_system_stop_module(idcu_ModuleSystem* system, 
                                    const char* name);
int idcu_module_system_destroy_module(idcu_ModuleSystem* system, 
                                       const char* name);

// Batch operations
int idcu_module_system_init_all(idcu_ModuleSystem* system);
int idcu_module_system_start_all(idcu_ModuleSystem* system);
int idcu_module_system_stop_all(idcu_ModuleSystem* system);
int idcu_module_system_destroy_all(idcu_ModuleSystem* system);
```

### Query

```c
// Find module by name
int idcu_module_system_find_by_name(idcu_ModuleSystem* system,
                                     const char* name,
                                     idcu_Module** out_module);

// Get all modules
int idcu_module_system_get_all(idcu_ModuleSystem* system,
                                idcu_ModuleInfo** out_infos,
                                size_t* out_count);

// Get modules by category
int idcu_module_system_get_by_category(idcu_ModuleSystem* system,
                                         const char* category,
                                         idcu_ModuleInfo** out_infos,
                                         size_t* out_count);
```

## Module Categories

Modules should be categorized for organization:

- `core` - Core system modules
- `network` - Networking-related modules
- `storage` - Storage and persistence modules
- `monitoring` - Metrics and health check modules
- `api` - API and interface modules
- `business` - Business logic modules
- `integration` - Integration modules

## Example Module

```c
#include <idcu/module/module.h>
#include <idcu/log/log.h>

static idcu_ErrorCode my_module_init(void) {
    IDCU_LOG_INFO("My module initializing...");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_start(void) {
    IDCU_LOG_INFO("My module starting...");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_stop(void) {
    IDCU_LOG_INFO("My module stopping...");
    return IDCU_ERR_OK;
}

static idcu_ErrorCode my_module_destroy(void) {
    IDCU_LOG_INFO("My module destroying...");
    return IDCU_ERR_OK;
}

static const char* dependencies[] = {"core-module"};

const idcu_ModuleDef my_module = {
    .name = "my-module",
    .version_str = "1.0.0",
    .description = "My example module",
    .category = "business",
    .init = my_module_init,
    .start = my_module_start,
    .stop = my_module_stop,
    .destroy = my_module_destroy,
    .dependencies = dependencies,
    .dependency_count = 1
};
```

## Best Practices

1. **Keep modules small and focused** - Single responsibility principle
2. **Declare all dependencies** - Explicit is better than implicit
3. **Handle errors gracefully** - Return appropriate error codes
4. **Log state transitions** - Help with debugging
5. **Clean up in destroy** - Release all resources
6. **Be idempotent** - Handle repeated init/destroy calls
