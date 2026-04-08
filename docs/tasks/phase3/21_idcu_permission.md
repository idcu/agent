# 任务 3.21: idcu-permission - 权限管理库

## 目标

创建权限管理库，支持：
- 角色管理
- 权限管理
- 用户-角色关联
- 角色-权限关联
- 权限检查
- 继承权限
- 权限缓存
- 权限审计

## 详细步骤

### 1. 创建目录结构

```bash
mkdir -p libs/idcu-permission/include/idcu/permission
mkdir -p libs/idcu-permission/src/idcu/permission
mkdir -p libs/idcu-permission/tests
mkdir -p libs/idcu-permission/examples
```

### 2. 创建权限管理头文件 (permission.h)

创建 `libs/idcu-permission/include/idcu/permission/permission.h`：

```c
#ifndef IDCU_PERMISSION_PERMISSION_H
#define IDCU_PERMISSION_PERMISSION_H

#include "idcu/common/error_code.h"
#include "idcu/common/vector.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t idcu_RoleId;
typedef uint64_t idcu_PermissionId;
typedef uint64_t idcu_SubjectId;

typedef enum
{
    IDCU_PERMISSION_ACTION_DENY = 0,
    IDCU_PERMISSION_ACTION_ALLOW,
    IDCU_PERMISSION_ACTION_AUDIT
} idcu_PermissionAction;

typedef enum
{
    IDCU_PERMISSION_EFFECT_DENY = 0,
    IDCU_PERMISSION_EFFECT_ALLOW
} idcu_PermissionEffect;

typedef struct
{
    idcu_PermissionId id;
    char name[128];
    char resource[256];
    char action[64];
    idcu_PermissionEffect effect;
    char description[512];
    char conditions[1024];
} idcu_Permission;

typedef struct
{
    idcu_RoleId id;
    char name[128];
    char description[512];
    idcu_Vector permissions;
    idcu_Vector parent_roles;
    int is_system;
} idcu_Role;

typedef struct
{
    idcu_SubjectId id;
    char name[128];
    char type[64];
    idcu_Vector roles;
    idcu_Vector direct_permissions;
} idcu_Subject;

typedef struct
{
    idcu_Vector roles;
    idcu_HashMap roles_by_id;
    idcu_HashMap roles_by_name;
    
    idcu_Vector permissions;
    idcu_HashMap permissions_by_id;
    idcu_HashMap permissions_by_name;
    
    idcu_Vector subjects;
    idcu_HashMap subjects_by_id;
    idcu_HashMap subjects_by_name;
    
    idcu_HashMap permission_cache;
    idcu_Mutex lock;
    
    int audit_enabled;
    idcu_Vector audit_log;
    
    int initialized;
} idcu_PermissionManager;

typedef struct
{
    int enable_cache;
    int enable_audit;
} idcu_PermissionManagerConfig;

int  idcu_permission_manager_config_init(idcu_PermissionManagerConfig* config);

int  idcu_permission_manager_init(idcu_PermissionManager* manager, const idcu_PermissionManagerConfig* config);
void idcu_permission_manager_destroy(idcu_PermissionManager* manager);

idcu_RoleId idcu_permission_manager_add_role(idcu_PermissionManager* manager, const char* name, const char* description);
int  idcu_permission_manager_remove_role(idcu_PermissionManager* manager, idcu_RoleId id);
idcu_Role* idcu_permission_manager_get_role(idcu_PermissionManager* manager, idcu_RoleId id);
idcu_Role* idcu_permission_manager_get_role_by_name(idcu_PermissionManager* manager, const char* name);
int  idcu_permission_manager_add_role_permission(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_PermissionId permission_id);
int  idcu_permission_manager_remove_role_permission(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_PermissionId permission_id);
int  idcu_permission_manager_add_role_parent(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_RoleId parent_id);
int  idcu_permission_manager_remove_role_parent(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_RoleId parent_id);
int  idcu_permission_manager_set_system_role(idcu_PermissionManager* manager, idcu_RoleId id, int is_system);

idcu_PermissionId idcu_permission_manager_add_permission(idcu_PermissionManager* manager, const char* name, const char* resource,
                                                        const char* action, idcu_PermissionEffect effect);
int  idcu_permission_manager_remove_permission(idcu_PermissionManager* manager, idcu_PermissionId id);
idcu_Permission* idcu_permission_manager_get_permission(idcu_PermissionManager* manager, idcu_PermissionId id);
idcu_Permission* idcu_permission_manager_get_permission_by_name(idcu_PermissionManager* manager, const char* name);
int  idcu_permission_manager_set_permission_description(idcu_PermissionManager* manager, idcu_PermissionId id, const char* description);
int  idcu_permission_manager_set_permission_conditions(idcu_PermissionManager* manager, idcu_PermissionId id, const char* conditions);

idcu_SubjectId idcu_permission_manager_add_subject(idcu_PermissionManager* manager, const char* name, const char* type);
int  idcu_permission_manager_remove_subject(idcu_PermissionManager* manager, idcu_SubjectId id);
idcu_Subject* idcu_permission_manager_get_subject(idcu_PermissionManager* manager, idcu_SubjectId id);
idcu_Subject* idcu_permission_manager_get_subject_by_name(idcu_PermissionManager* manager, const char* name);
int  idcu_permission_manager_assign_role(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_RoleId role_id);
int  idcu_permission_manager_revoke_role(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_RoleId role_id);
int  idcu_permission_manager_assign_direct_permission(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_PermissionId permission_id);
int  idcu_permission_manager_revoke_direct_permission(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_PermissionId permission_id);

int  idcu_permission_manager_check(idcu_PermissionManager* manager, idcu_SubjectId subject_id, const char* resource, const char* action);
int  idcu_permission_manager_check_with_context(idcu_PermissionManager* manager, idcu_SubjectId subject_id,
                                                 const char* resource, const char* action, const char* context);
int  idcu_permission_manager_has_role(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_RoleId role_id);
int  idcu_permission_manager_has_permission(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_PermissionId permission_id);

int  idcu_permission_manager_get_subject_roles(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_Vector* roles);
int  idcu_permission_manager_get_subject_permissions(idcu_PermissionManager* manager, idcu_SubjectId subject_id, idcu_Vector* permissions);
int  idcu_permission_manager_get_role_permissions(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_Vector* permissions, int include_inherited);

int  idcu_permission_manager_clear_cache(idcu_PermissionManager* manager);
int  idcu_permission_manager_invalidate_subject_cache(idcu_PermissionManager* manager, idcu_SubjectId subject_id);
int  idcu_permission_manager_invalidate_role_cache(idcu_PermissionManager* manager, idcu_RoleId role_id);

int  idcu_permission_manager_enable_audit(idcu_PermissionManager* manager);
int  idcu_permission_manager_disable_audit(idcu_PermissionManager* manager);
int  idcu_permission_manager_get_audit_log(idcu_PermissionManager* manager, idcu_Vector* log, size_t limit);
int  idcu_permission_manager_clear_audit_log(idcu_PermissionManager* manager);

int  idcu_permission_manager_save(idcu_PermissionManager* manager, const char* path);
int  idcu_permission_manager_load(idcu_PermissionManager* manager, const char* path);
int  idcu_permission_manager_export_json(idcu_PermissionManager* manager, char* buffer, size_t buffer_size);
int  idcu_permission_manager_import_json(idcu_PermissionManager* manager, const char* json);

int  idcu_role_init(idcu_Role* role, const char* name, const char* description);
void idcu_role_destroy(idcu_Role* role);

int  idcu_permission_init(idcu_Permission* permission, const char* name, const char* resource,
                           const char* action, idcu_PermissionEffect effect);
void idcu_permission_destroy(idcu_Permission* permission);

int  idcu_subject_init(idcu_Subject* subject, const char* name, const char* type);
void idcu_subject_destroy(idcu_Subject* subject);

const char* idcu_permission_effect_to_string(idcu_PermissionEffect effect);

#ifdef __cplusplus
}
#endif

#endif
```

### 3. 创建 CMakeLists.txt

创建 `libs/idcu-permission/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.15)
project(idcu-permission VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(idcu-permission STATIC
    src/idcu/permission/permission.c
)

target_include_directories(idcu-permission PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(idcu-permission PRIVATE
    idcu::common
    idcu::storage
    idcu::json
    idcu::log
)

add_library(idcu::permission ALIAS idcu-permission)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

### 4. 创建模块配置文件 (module.yaml)

创建 `libs/idcu-permission/module.yaml`：

```yaml
name: idcu-permission
version: 1.0.0
description: Permission management library for IDCU Agent
author: IDCU Team
license: MIT

dependencies:
  - idcu-common
  - idcu-storage
  - idcu-json
  - idcu-log

build:
  type: cmake
  targets:
    - idcu-permission

headers:
  - idcu/permission/permission.h

features:
  - roles: Role management
  - permissions: Permission management
  - assignment: Subject-role assignment
  - role_permission: Role-permission assignment
  - check: Permission checking
  - inheritance: Inherited permissions
  - cache: Permission cache
  - audit: Permission audit
  - persistence: Permission persistence

testing:
  enabled: true
  framework: internal
```

### 5. 创建 README.md

创建 `libs/idcu-permission/README.md`：

```markdown
# idcu-permission

IDCU Agent 的权限管理库。

## 功能特性

- **角色管理**: 角色管理
- **权限管理**: 权限管理
- **用户-角色关联**: 用户-角色关联
- **角色-权限关联**: 角色-权限关联
- **权限检查**: 权限检查
- **继承权限**: 继承权限
- **权限缓存**: 权限缓存
- **权限审计**: 权限审计
- **持久化**: 权限持久化

## 快速开始

### 初始化权限管理器

```c
#include "idcu/permission/permission.h"

idcu_PermissionManagerConfig config;
idcu_permission_manager_config_init(&config);

config.enable_cache = 1;
config.enable_audit = 1;

idcu_PermissionManager manager;
idcu_permission_manager_init(&manager, &config);
```

### 创建权限

```c
idcu_PermissionId read_perm = idcu_permission_manager_add_permission(
    &manager, "read_file", "/api/files", "read", IDCU_PERMISSION_EFFECT_ALLOW
);

idcu_PermissionId write_perm = idcu_permission_manager_add_permission(
    &manager, "write_file", "/api/files", "write", IDCU_PERMISSION_EFFECT_ALLOW
);

idcu_PermissionId delete_perm = idcu_permission_manager_add_permission(
    &manager, "delete_file", "/api/files", "delete", IDCU_PERMISSION_EFFECT_ALLOW
);
```

### 创建角色

```c
idcu_RoleId viewer_role = idcu_permission_manager_add_role(
    &manager, "viewer", "Can view files"
);

idcu_RoleId editor_role = idcu_permission_manager_add_role(
    &manager, "editor", "Can view and edit files"
);

idcu_RoleId admin_role = idcu_permission_manager_add_role(
    &manager, "admin", "Full access"
);
```

### 为角色分配权限

```c
idcu_permission_manager_add_role_permission(&manager, viewer_role, read_perm);

idcu_permission_manager_add_role_permission(&manager, editor_role, read_perm);
idcu_permission_manager_add_role_permission(&manager, editor_role, write_perm);

idcu_permission_manager_add_role_permission(&manager, admin_role, read_perm);
idcu_permission_manager_add_role_permission(&manager, admin_role, write_perm);
idcu_permission_manager_add_role_permission(&manager, admin_role, delete_perm);
```

### 角色继承

```c
idcu_permission_manager_add_role_parent(&manager, editor_role, viewer_role);
idcu_permission_manager_add_role_parent(&manager, admin_role, editor_role);
```

### 创建用户（主体）

```c
idcu_SubjectId user1 = idcu_permission_manager_add_subject(
    &manager, "john", "user"
);

idcu_SubjectId user2 = idcu_permission_manager_add_subject(
    &manager, "jane", "user"
);

idcu_SubjectId admin = idcu_permission_manager_add_subject(
    &manager, "admin", "user"
);
```

### 为用户分配角色

```c
idcu_permission_manager_assign_role(&manager, user1, viewer_role);
idcu_permission_manager_assign_role(&manager, user2, editor_role);
idcu_permission_manager_assign_role(&manager, admin, admin_role);
```

### 直接权限分配

```c
idcu_permission_manager_assign_direct_permission(&manager, user1, delete_perm);
```

### 权限检查

```c
if (idcu_permission_manager_check(&manager, user1, "/api/files", "read") == IDCU_ERR_OK) {
    printf("User 1 can read files\n");
}

if (idcu_permission_manager_check(&manager, user2, "/api/files", "write") == IDCU_ERR_OK) {
    printf("User 2 can write files\n");
}

if (idcu_permission_manager_check(&manager, admin, "/api/files", "delete") == IDCU_ERR_OK) {
    printf("Admin can delete files\n");
}
```

### 检查角色

```c
if (idcu_permission_manager_has_role(&manager, user1, viewer_role)) {
    printf("User 1 has viewer role\n");
}
```

### 获取用户的角色

```c
idcu_Vector roles;
idcu_vector_init(&roles, sizeof(idcu_RoleId));

idcu_permission_manager_get_subject_roles(&manager, user2, &roles);

for (size_t i = 0; i < roles.count; i++) {
    idcu_RoleId* role_id = (idcu_RoleId*)idcu_vector_get(&roles, i);
    idcu_Role* role = idcu_permission_manager_get_role(&manager, *role_id);
    printf("Role: %s\n", role->name);
}

idcu_vector_destroy(&roles);
```

### 获取用户的所有权限

```c
idcu_Vector permissions;
idcu_vector_init(&permissions, sizeof(idcu_PermissionId));

idcu_permission_manager_get_subject_permissions(&manager, user2, &permissions);

for (size_t i = 0; i < permissions.count; i++) {
    idcu_PermissionId* perm_id = (idcu_PermissionId*)idcu_vector_get(&permissions, i);
    idcu_Permission* perm = idcu_permission_manager_get_permission(&manager, *perm_id);
    printf("Permission: %s on %s\n", perm->action, perm->resource);
}

idcu_vector_destroy(&permissions);
```

### 缓存管理

```c
idcu_permission_manager_invalidate_subject_cache(&manager, user1);
idcu_permission_manager_invalidate_role_cache(&manager, editor_role);
idcu_permission_manager_clear_cache(&manager);
```

### 审计日志

```c
idcu_Vector audit_log;
idcu_vector_init(&audit_log, sizeof(idcu_PermissionAuditEntry));

idcu_permission_manager_get_audit_log(&manager, &audit_log, 100);

for (size_t i = 0; i < audit_log.count; i++) {
    // Process audit entries
}

idcu_permission_manager_clear_audit_log(&manager);
idcu_vector_destroy(&audit_log);
```

### 持久化

```c
idcu_permission_manager_save(&manager, "./permissions.dat");
idcu_permission_manager_load(&manager, "./permissions.dat");

char json_buffer[8192];
idcu_permission_manager_export_json(&manager, json_buffer, sizeof(json_buffer));

idcu_permission_manager_import_json(&manager, json_buffer);
```

### 清理

```c
idcu_permission_manager_destroy(&manager);
```

## 权限效果

| 效果 | 说明 |
|-----|------|
| DENY | 拒绝 |
| ALLOW | 允许 |

## API 文档

详见 [include/idcu/permission/permission.h](include/idcu/permission/permission.h)
```

## 验证检查清单

- [ ] 权限管理头文件已创建
- [ ] 权限管理实现文件已创建
- [ ] CMakeLists.txt 已创建
- [ ] module.yaml 配置文件已创建
- [ ] README.md 已创建
- [ ] 可以创建角色和权限
- [ ] 权限检查正常工作
- [ ] 角色继承正常工作

## Git 提交

```bash
git add libs/idcu-permission/
git commit -m "feat: add idcu-permission library

- Add role management
- Add permission management
- Add subject-role assignment
- Add role-permission assignment
- Add permission checking
- Add inherited permissions
- Add permission cache
- Add permission audit
- Add permission persistence
- Add CMake build configuration
- Add module.yaml metadata"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 权限检查失败 | 角色或权限未正确分配 | 检查角色和权限分配 |
| 继承不生效 | 父子关系未正确设置 | 确保角色继承关系正确 |
| 缓存不一致 | 权限变更后未清除缓存 | 清除相关缓存 |
