#ifndef IDCU_PERMISSION_PERMISSION_H
#define IDCU_PERMISSION_PERMISSION_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include "idcu/permission/types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int idcu_permission_manager_init(idcu_Permission_Context** ctx);
void idcu_permission_manager_shutdown(idcu_Permission_Context* ctx);

int idcu_permission_check(idcu_Permission_Context* ctx, const char* module_name, const char* permission);
int idcu_permission_check_any(idcu_Permission_Context* ctx, const char* module_name, const char** permissions, int count);
int idcu_permission_check_all(idcu_Permission_Context* ctx, const char* module_name, const char** permissions, int count);

int idcu_permission_grant(idcu_Permission_Context* ctx, const char* module_name, const char* permission);
int idcu_permission_revoke(idcu_Permission_Context* ctx, const char* module_name, const char* permission);

int idcu_permission_get_module_permissions(idcu_Permission_Context* ctx, const char* module_name, char** out_permissions, int* out_count);
int idcu_permission_list_modules(idcu_Permission_Context* ctx, char** out_modules, int* out_count);

#ifdef __cplusplus
}
#endif

#endif
