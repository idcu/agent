#ifndef IDCU_PERMISSION_TYPES_H
#define IDCU_PERMISSION_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MAX_PERMISSIONS        256
#define IDCU_PERMISSION_NAME_MAX    128
#define IDCU_MAX_MODULE_PERMISSIONS 64

typedef struct
{
    char module_name[64];
    char permissions[IDCU_MAX_MODULE_PERMISSIONS][IDCU_PERMISSION_NAME_MAX];
    int  permission_count;
} idcu_ModulePermission;

typedef struct
{
    idcu_ModulePermission module_perms[IDCU_MAX_MODULE_PERMISSIONS];
    int                   module_count;
    idcu_Mutex            lock;
    int                   initialized;
    uint64_t              operation_count;
    uint64_t              error_count;
} idcu_PermissionManager;

typedef idcu_PermissionManager idcu_Permission_Context;

#ifdef __cplusplus
}
#endif

#endif
