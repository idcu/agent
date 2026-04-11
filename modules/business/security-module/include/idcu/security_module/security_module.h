#ifndef IDCU_SECURITY_MODULE_SECURITY_MODULE_H
#define IDCU_SECURITY_MODULE_SECURITY_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/common/security.h>
#include <idcu/sandbox/sandbox.h>
#include <idcu/permission/permission.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_Sandbox* sandbox;
    idcu_PermissionManager* permission_manager;
    int initialized;
    void* user_data;
} idcu_SecurityModule;

int idcu_security_module_init(idcu_SecurityModule* sm);
int idcu_security_module_start(idcu_SecurityModule* sm);
int idcu_security_module_stop(idcu_SecurityModule* sm);
void idcu_security_module_destroy(idcu_SecurityModule* sm);

int idcu_security_module_check_permission(idcu_SecurityModule* sm, const char* permission);
int idcu_security_module_encrypt(idcu_SecurityModule* sm, const void* input, size_t input_size, void* output, size_t* output_size);
int idcu_security_module_decrypt(idcu_SecurityModule* sm, const void* input, size_t input_size, void* output, size_t* output_size);

idcu_Sandbox* idcu_security_module_get_sandbox(idcu_SecurityModule* sm);
idcu_PermissionManager* idcu_security_module_get_permission_manager(idcu_SecurityModule* sm);

#ifdef __cplusplus
}
#endif

#endif
