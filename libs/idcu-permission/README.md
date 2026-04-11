# idcu-permission

IDCU Agent 的权限管理库。

## 功能特性
- 模块级权限管理
- 通配符权限支持 (file.*)
- 权限检查、授予和撤销
- 批量权限检查（任意/全部）
- 线程安全实现
- CMake 构建配置

## 使用方法

```c
#include <idcu/permission/permission.h>

idcu_PermissionManager* ctx;
int ret = idcu_permission_manager_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // 授予权限
    idcu_permission_grant(ctx, "file", "read");
    idcu_permission_grant(ctx, "file", "write");
    
    // 检查权限
    if (idcu_permission_check(ctx, "file", "read") == IDCU_ERR_OK) {
        // 权限已授予
    }
    
    // 关闭
    idcu_permission_manager_shutdown(ctx);
}
```

## API 参考
- `idcu_permission_manager_init()` - 初始化权限管理器
- `idcu_permission_manager_shutdown()` - 关闭权限管理器
- `idcu_permission_check()` - 检查单个权限
- `idcu_permission_check_any()` - 检查列表中的任意权限
- `idcu_permission_check_all()` - 检查列表中的所有权限
- `idcu_permission_grant()` - 授予权限
- `idcu_permission_revoke()` - 撤销权限
- `idcu_permission_get_module_permissions()` - 获取模块的所有权限
- `idcu_permission_list_modules()` - 列出所有有权限的模块

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
