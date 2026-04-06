#include "idcu/permission/permission.h"
#include <string.h>
#include <stdlib.h>

static idcu_PermissionManager g_perm_mgr;
static int g_initialized = 0;

static idcu_ModulePermission* find_module_perm(const char* module_name)
{
    if (!module_name) {
        return NULL;
    }
    for (int i = 0; i < g_perm_mgr.module_count; i++) {
        if (strcmp(g_perm_mgr.module_perms[i].module_name, module_name) == 0) {
            return &g_perm_mgr.module_perms[i];
        }
    }
    return NULL;
}

static int has_permission(idcu_ModulePermission* mod_perm, const char* permission)
{
    if (!mod_perm || !permission) {
        return 0;
    }
    for (int i = 0; i < mod_perm->permission_count; i++) {
        if (strcmp(mod_perm->permissions[i], permission) == 0) {
            return 1;
        }
        char perm_prefix[IDCU_PERMISSION_NAME_MAX];
        const char* dot = strchr(permission, '.');
        if (dot) {
            size_t prefix_len = dot - permission;
            if (prefix_len < sizeof(perm_prefix)) {
                strncpy(perm_prefix, permission, prefix_len);
                perm_prefix[prefix_len] = '\0';
                if (strcmp(mod_perm->permissions[i], perm_prefix) == 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int idcu_permission_manager_init(void)
{
    if (g_initialized) {
        idcu_permission_manager_shutdown();
    }
    memset(&g_perm_mgr, 0, sizeof(idcu_PermissionManager));
    int ret = idcu_mutex_init(&g_perm_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    g_initialized = 1;
    g_perm_mgr.initialized = 1;
    return IDCU_ERR_SUCCESS;
}

void idcu_permission_manager_shutdown(void)
{
    if (!g_initialized) {
        return;
    }
    idcu_mutex_destroy(&g_perm_mgr.lock);
    memset(&g_perm_mgr, 0, sizeof(idcu_PermissionManager));
    g_initialized = 0;
}

int idcu_permission_check(const char* module_name, const char* permission)
{
    if (!g_initialized || !module_name || !permission) {
        return 0;
    }
    int ret = idcu_mutex_lock(&g_perm_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return 0;
    }
    idcu_ModulePermission* mod_perm = find_module_perm(module_name);
    int result = has_permission(mod_perm, permission);
    idcu_mutex_unlock(&g_perm_mgr.lock);
    return result;
}

int idcu_permission_check_any(const char* module_name, const char** permissions, int count)
{
    if (!g_initialized || !module_name || !permissions || count <= 0) {
        return 0;
    }
    for (int i = 0; i < count; i++) {
        if (idcu_permission_check(module_name, permissions[i])) {
            return 1;
        }
    }
    return 0;
}

int idcu_permission_check_all(const char* module_name, const char** permissions, int count)
{
    if (!g_initialized || !module_name || !permissions || count <= 0) {
        return 0;
    }
    for (int i = 0; i < count; i++) {
        if (!idcu_permission_check(module_name, permissions[i])) {
            return 0;
        }
    }
    return 1;
}

int idcu_permission_grant(const char* module_name, const char* permission)
{
    if (!g_initialized || !module_name || !permission) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_perm_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_ModulePermission* mod_perm = find_module_perm(module_name);
    if (!mod_perm) {
        if (g_perm_mgr.module_count >= IDCU_MAX_MODULE_PERMISSIONS) {
            idcu_mutex_unlock(&g_perm_mgr.lock);
            return IDCU_ERR_QUEUE_FULL;
        }
        mod_perm = &g_perm_mgr.module_perms[g_perm_mgr.module_count];
        strncpy(mod_perm->module_name, module_name, sizeof(mod_perm->module_name) - 1);
        mod_perm->module_name[sizeof(mod_perm->module_name) - 1] = '\0';
        mod_perm->permission_count = 0;
        g_perm_mgr.module_count++;
    }
    if (has_permission(mod_perm, permission)) {
        idcu_mutex_unlock(&g_perm_mgr.lock);
        return IDCU_ERR_ALREADY_EXISTS;
    }
    if (mod_perm->permission_count >= IDCU_MAX_MODULE_PERMISSIONS) {
        idcu_mutex_unlock(&g_perm_mgr.lock);
        return IDCU_ERR_QUEUE_FULL;
    }
    strncpy(mod_perm->permissions[mod_perm->permission_count], 
            permission, IDCU_PERMISSION_NAME_MAX - 1);
    mod_perm->permissions[mod_perm->permission_count][IDCU_PERMISSION_NAME_MAX - 1] = '\0';
    mod_perm->permission_count++;
    idcu_mutex_unlock(&g_perm_mgr.lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_permission_revoke(const char* module_name, const char* permission)
{
    if (!g_initialized || !module_name || !permission) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_perm_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_ModulePermission* mod_perm = find_module_perm(module_name);
    if (!mod_perm) {
        idcu_mutex_unlock(&g_perm_mgr.lock);
        return IDCU_ERR_NOT_FOUND;
    }
    for (int i = 0; i < mod_perm->permission_count; i++) {
        if (strcmp(mod_perm->permissions[i], permission) == 0) {
            if (i < mod_perm->permission_count - 1) {
                memmove(&mod_perm->permissions[i], &mod_perm->permissions[i + 1],
                        (mod_perm->permission_count - i - 1) * sizeof(mod_perm->permissions[0]));
            }
            mod_perm->permission_count--;
            idcu_mutex_unlock(&g_perm_mgr.lock);
            return IDCU_ERR_SUCCESS;
        }
    }
    idcu_mutex_unlock(&g_perm_mgr.lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_permission_get_module_permissions(const char* module_name, char** out_permissions, int* out_count)
{
    if (!g_initialized || !module_name || !out_permissions || !out_count) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_perm_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    idcu_ModulePermission* mod_perm = find_module_perm(module_name);
    if (!mod_perm) {
        *out_count = 0;
        idcu_mutex_unlock(&g_perm_mgr.lock);
        return IDCU_ERR_SUCCESS;
    }
    *out_count = mod_perm->permission_count;
    for (int i = 0; i < mod_perm->permission_count; i++) {
        out_permissions[i] = mod_perm->permissions[i];
    }
    idcu_mutex_unlock(&g_perm_mgr.lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_permission_list_modules(char** out_modules, int* out_count)
{
    if (!g_initialized || !out_modules || !out_count) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int ret = idcu_mutex_lock(&g_perm_mgr.lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }
    *out_count = g_perm_mgr.module_count;
    for (int i = 0; i < g_perm_mgr.module_count; i++) {
        out_modules[i] = g_perm_mgr.module_perms[i].module_name;
    }
    idcu_mutex_unlock(&g_perm_mgr.lock);
    return IDCU_ERR_SUCCESS;
}
