#include <idcu/security_module/security_module.h>
#include <string.h>

int idcu_security_module_init(idcu_SecurityModule* sm) {
    if (!sm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(sm, 0, sizeof(idcu_SecurityModule));
    sm->initialized = 0;
    return IDCU_ERR_OK;
}

int idcu_security_module_start(idcu_SecurityModule* sm) {
    if (!sm) {
        return IDCU_ERR_INVALID_ARG;
    }

    sm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_security_module_stop(idcu_SecurityModule* sm) {
    if (!sm) {
        return IDCU_ERR_INVALID_ARG;
    }

    sm->initialized = 0;
    return IDCU_ERR_OK;
}

void idcu_security_module_destroy(idcu_SecurityModule* sm) {
    if (!sm) {
        return;
    }

    if (sm->initialized) {
        idcu_security_module_stop(sm);
    }

    memset(sm, 0, sizeof(idcu_SecurityModule));
}

int idcu_security_module_check_permission(idcu_SecurityModule* sm, const char* permission) {
    if (!sm || !sm->initialized || !permission) {
        return IDCU_ERR_INVALID_STATE;
    }
    (void)permission;
    return IDCU_ERR_OK;
}

int idcu_security_module_encrypt(idcu_SecurityModule* sm, const void* input, size_t input_size, void* output, size_t* output_size) {
    if (!sm || !sm->initialized || !input || !output || !output_size) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)input;
    (void)input_size;
    *output_size = 0;
    return IDCU_ERR_OK;
}

int idcu_security_module_decrypt(idcu_SecurityModule* sm, const void* input, size_t input_size, void* output, size_t* output_size) {
    if (!sm || !sm->initialized || !input || !output || !output_size) {
        return IDCU_ERR_INVALID_ARG;
    }
    (void)input;
    (void)input_size;
    *output_size = 0;
    return IDCU_ERR_OK;
}

idcu_Sandbox* idcu_security_module_get_sandbox(idcu_SecurityModule* sm) {
    if (!sm) {
        return NULL;
    }
    return sm->sandbox;
}

idcu_PermissionManager* idcu_security_module_get_permission_manager(idcu_SecurityModule* sm) {
    if (!sm) {
        return NULL;
    }
    return sm->permission_manager;
}
