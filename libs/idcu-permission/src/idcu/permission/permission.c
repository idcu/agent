#include "idcu/permission/permission.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int wildcard_match(const char* pattern, const char* string)
{
    const char* cp = NULL;
    const char* mp = NULL;

    while (*string && *pattern != '*') {
        if (*pattern != *string) {
            return 0;
        }
        pattern++;
        string++;
    }

    while (*string) {
        if (*pattern == '*') {
            if (!*++pattern) {
                return 1;
            }
            mp = pattern;
            cp = string + 1;
        } else if (*pattern == *string) {
            pattern++;
            string++;
        } else {
            pattern = mp;
            string = cp++;
        }
    }

    while (*pattern == '*') {
        pattern++;
    }

    return !*pattern;
}

static idcu_ModulePermission* find_module(idcu_Permission_Context* ctx, const char* module_name)
{
    for (int i = 0; i < ctx->module_count; i++) {
        if (strcmp(ctx->module_perms[i].module_name, module_name) == 0) {
            return &ctx->module_perms[i];
        }
    }
    return NULL;
}

static int find_permission_index(idcu_ModulePermission* module, const char* permission)
{
    for (int i = 0; i < module->permission_count; i++) {
        if (wildcard_match(module->permissions[i], permission)) {
            return i;
        }
    }
    return -1;
}

int idcu_permission_manager_init(idcu_Permission_Context** ctx)
{
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }

    *ctx = (idcu_Permission_Context*)calloc(1, sizeof(idcu_Permission_Context));
    if (!*ctx) {
        return IDCU_ERR_MEMORY;
    }

    idcu_mutex_init(&(*ctx)->lock);
    (*ctx)->initialized = 1;
    (*ctx)->operation_count = 0;
    (*ctx)->error_count = 0;

    return IDCU_ERR_OK;
}

void idcu_permission_manager_shutdown(idcu_Permission_Context* ctx)
{
    if (!ctx) {
        return;
    }

    idcu_mutex_lock(&ctx->lock);
    idcu_Mutex lock_copy = ctx->lock;
    ctx->initialized = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);

    free(ctx);
}

int idcu_permission_check(idcu_Permission_Context* ctx, const char* module_name, const char* permission)
{
    if (!ctx || !module_name || !permission) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_ModulePermission* module = find_module(ctx, module_name);
    if (!module) {
        idcu_mutex_unlock(&ctx->lock);
        return IDCU_ERR_PERM_DENIED;
    }

    int result = IDCU_ERR_PERM_DENIED;
    if (find_permission_index(module, permission) >= 0) {
        result = IDCU_ERR_OK;
    }

    idcu_mutex_unlock(&ctx->lock);
    return result;
}

int idcu_permission_check_any(idcu_Permission_Context* ctx, const char* module_name, const char** permissions, int count)
{
    if (!ctx || !module_name || !permissions || count <= 0) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_ModulePermission* module = find_module(ctx, module_name);
    if (!module) {
        idcu_mutex_unlock(&ctx->lock);
        return IDCU_ERR_PERM_DENIED;
    }

    int result = IDCU_ERR_PERM_DENIED;
    for (int i = 0; i < count; i++) {
        if (find_permission_index(module, permissions[i]) >= 0) {
            result = IDCU_ERR_OK;
            break;
        }
    }

    idcu_mutex_unlock(&ctx->lock);
    return result;
}

int idcu_permission_check_all(idcu_Permission_Context* ctx, const char* module_name, const char** permissions, int count)
{
    if (!ctx || !module_name || !permissions || count <= 0) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_ModulePermission* module = find_module(ctx, module_name);
    if (!module) {
        idcu_mutex_unlock(&ctx->lock);
        return IDCU_ERR_PERM_DENIED;
    }

    int result = IDCU_ERR_OK;
    for (int i = 0; i < count; i++) {
        if (find_permission_index(module, permissions[i]) < 0) {
            result = IDCU_ERR_PERM_DENIED;
            break;
        }
    }

    idcu_mutex_unlock(&ctx->lock);
    return result;
}

int idcu_permission_grant(idcu_Permission_Context* ctx, const char* module_name, const char* permission)
{
    if (!ctx || !module_name || !permission) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    if (strlen(module_name) >= 64 || strlen(permission) >= IDCU_PERMISSION_NAME_MAX) {
        ctx->error_count++;
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_ModulePermission* module = find_module(ctx, module_name);
    if (!module) {
        if (ctx->module_count >= IDCU_MAX_MODULE_PERMISSIONS) {
            idcu_mutex_unlock(&ctx->lock);
            ctx->error_count++;
            return IDCU_ERR_LIMIT_EXCEEDED;
        }
        module = &ctx->module_perms[ctx->module_count++];
        strncpy(module->module_name, module_name, 63);
        module->module_name[63] = '\0';
        module->permission_count = 0;
    }

    if (find_permission_index(module, permission) >= 0) {
        idcu_mutex_unlock(&ctx->lock);
        return IDCU_ERR_OK;
    }

    if (module->permission_count >= IDCU_MAX_MODULE_PERMISSIONS) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_LIMIT_EXCEEDED;
    }

    strncpy(module->permissions[module->permission_count], permission, IDCU_PERMISSION_NAME_MAX - 1);
    module->permissions[module->permission_count][IDCU_PERMISSION_NAME_MAX - 1] = '\0';
    module->permission_count++;

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_permission_revoke(idcu_Permission_Context* ctx, const char* module_name, const char* permission)
{
    if (!ctx || !module_name || !permission) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_ModulePermission* module = find_module(ctx, module_name);
    if (!module) {
        idcu_mutex_unlock(&ctx->lock);
        return IDCU_ERR_OK;
    }

    int index = -1;
    for (int i = 0; i < module->permission_count; i++) {
        if (strcmp(module->permissions[i], permission) == 0) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        for (int i = index; i < module->permission_count - 1; i++) {
            strncpy(module->permissions[i], module->permissions[i + 1], IDCU_PERMISSION_NAME_MAX - 1);
            module->permissions[i][IDCU_PERMISSION_NAME_MAX - 1] = '\0';
        }
        module->permission_count--;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_permission_get_module_permissions(idcu_Permission_Context* ctx, const char* module_name, char** out_permissions, int* out_count)
{
    if (!ctx || !module_name || !out_permissions || !out_count) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    idcu_ModulePermission* module = find_module(ctx, module_name);
    if (!module) {
        *out_count = 0;
        idcu_mutex_unlock(&ctx->lock);
        return IDCU_ERR_OK;
    }

    *out_count = module->permission_count;
    for (int i = 0; i < module->permission_count; i++) {
        out_permissions[i] = module->permissions[i];
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}

int idcu_permission_list_modules(idcu_Permission_Context* ctx, char** out_modules, int* out_count)
{
    if (!ctx || !out_modules || !out_count) {
        if (ctx) {
            ctx->error_count++;
        }
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_mutex_lock(&ctx->lock);
    ctx->operation_count++;

    if (!ctx->initialized) {
        idcu_mutex_unlock(&ctx->lock);
        ctx->error_count++;
        return IDCU_ERR_NOT_INITIALIZED;
    }

    *out_count = ctx->module_count;
    for (int i = 0; i < ctx->module_count; i++) {
        out_modules[i] = ctx->module_perms[i].module_name;
    }

    idcu_mutex_unlock(&ctx->lock);
    return IDCU_ERR_OK;
}
