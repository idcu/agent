# idcu-permission

Permission management library for IDCU Agent.

## Features
- Module-level permission management
- Wildcard permission support (file.*)
- Permission check, grant, and revoke
- Batch permission checks (any/all)
- Thread-safe implementation
- CMake build configuration

## Usage

```c
#include <idcu/permission/permission.h>

idcu_Permission_Context* ctx;
int ret = idcu_permission_manager_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Grant permissions
    idcu_permission_grant(ctx, "file", "read");
    idcu_permission_grant(ctx, "file", "write");
    
    // Check permission
    if (idcu_permission_check(ctx, "file", "read") == IDCU_ERR_OK) {
        // Permission granted
    }
    
    // Shutdown
    idcu_permission_manager_shutdown(ctx);
}
```

## API Reference
- `idcu_permission_manager_init()` - Initialize permission manager
- `idcu_permission_manager_shutdown()` - Shutdown permission manager
- `idcu_permission_check()` - Check single permission
- `idcu_permission_check_any()` - Check any permission in list
- `idcu_permission_check_all()` - Check all permissions in list
- `idcu_permission_grant()` - Grant permission
- `idcu_permission_revoke()` - Revoke permission
- `idcu_permission_get_module_permissions()` - Get all permissions for module
- `idcu_permission_list_modules()` - List all modules with permissions

## Building

```bash
cmake -B build && cmake --build build
```

## License
MIT
