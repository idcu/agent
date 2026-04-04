#ifndef IDCU_UTILS_PERMISSION_MANAGER_H
#define IDCU_UTILS_PERMISSION_MANAGER_H

#include "error_code.h"
#include "lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MAX_PERMISSIONS 256
#define IDCU_PERMISSION_NAME_MAX 128
#define IDCU_MAX_MODULE_PERMISSIONS 64

typedef struct {
    char module_name[64];
    char permissions[IDCU_MAX_MODULE_PERMISSIONS][IDCU_PERMISSION_NAME_MAX];
    int permission_count;
} idcu_ModulePermission;

typedef struct {
    idcu_ModulePermission module_perms[IDCU_MAX_MODULE_PERMISSIONS];
    int module_count;
    idcu_Mutex lock;
    int initialized;
} idcu_PermissionManager;

int idcu_permission_manager_init(void);
void idcu_permission_manager_shutdown(void);
int idcu_permission_load_from_config(const char* config_file);

int idcu_permission_check(const char* module_name, const char* permission);
int idcu_permission_check_any(const char* module_name, const char** permissions, int count);
int idcu_permission_check_all(const char* module_name, const char** permissions, int count);

int idcu_permission_grant(const char* module_name, const char* permission);
int idcu_permission_revoke(const char* module_name, const char* permission);

int idcu_permission_get_module_permissions(const char* module_name, char** out_permissions, int* out_count);
int idcu_permission_list_modules(char** out_modules, int* out_count);

#ifdef __cplusplus
}
#endif

#endif
