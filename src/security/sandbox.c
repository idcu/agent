#include "security/sandbox.h"
#include <string.h>

static idcu_Sandbox* g_sandbox_registry[IDCU_MAX_SANDBOXES] = {0};
static int g_sandbox_registry_initialized = 0;

int idcu_sandbox_init(idcu_Sandbox* sb, uint32_t module_id, uint32_t initial_perm) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(sb, 0, sizeof(idcu_Sandbox));
    sb->module_id = module_id;
    sb->perm = initial_perm;
    sb->quota = 0;
    
    return IDCU_ERR_OK;
}

void idcu_sandbox_destroy(idcu_Sandbox* sb) {
    if (!sb) {
        return;
    }
    memset(sb, 0, sizeof(idcu_Sandbox));
}

int idcu_sandbox_set_perm(idcu_Sandbox* sb, uint32_t perm) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->perm = perm;
    return IDCU_ERR_OK;
}

int idcu_sandbox_add_perm(idcu_Sandbox* sb, uint32_t perm) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->perm |= perm;
    return IDCU_ERR_OK;
}

int idcu_sandbox_remove_perm(idcu_Sandbox* sb, uint32_t perm) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->perm &= ~perm;
    return IDCU_ERR_OK;
}

int idcu_sandbox_perm_check(idcu_Sandbox* sb, uint32_t mask) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    return ((sb->perm & mask) == mask) ? IDCU_ERR_OK : IDCU_ERR_PERM_DENIED;
}

uint32_t idcu_sandbox_get_perm(idcu_Sandbox* sb) {
    if (!sb) {
        return 0;
    }
    return sb->perm;
}

int idcu_sandbox_set_quota(idcu_Sandbox* sb, uint32_t quota) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->quota = quota;
    return IDCU_ERR_OK;
}

uint32_t idcu_sandbox_get_quota(idcu_Sandbox* sb) {
    if (!sb) {
        return 0;
    }
    return sb->quota;
}

int idcu_sandbox_registry_init(void) {
    if (g_sandbox_registry_initialized) {
        return IDCU_ERR_OK;
    }
    
    memset(g_sandbox_registry, 0, sizeof(g_sandbox_registry));
    g_sandbox_registry_initialized = 1;
    
    return IDCU_ERR_OK;
}

void idcu_sandbox_registry_destroy(void) {
    if (!g_sandbox_registry_initialized) {
        return;
    }
    
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        if (g_sandbox_registry[i]) {
            idcu_sandbox_destroy(g_sandbox_registry[i]);
            g_sandbox_registry[i] = NULL;
        }
    }
    
    g_sandbox_registry_initialized = 0;
}

int idcu_sandbox_registry_add(idcu_Sandbox* sb) {
    if (!g_sandbox_registry_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        if (g_sandbox_registry[i] && g_sandbox_registry[i]->module_id == sb->module_id) {
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }
    
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        if (!g_sandbox_registry[i]) {
            g_sandbox_registry[i] = sb;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NO_MEMORY;
}

int idcu_sandbox_registry_remove(uint32_t module_id) {
    if (!g_sandbox_registry_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        if (g_sandbox_registry[i] && g_sandbox_registry[i]->module_id == module_id) {
            idcu_sandbox_destroy(g_sandbox_registry[i]);
            g_sandbox_registry[i] = NULL;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

idcu_Sandbox* idcu_sandbox_registry_get(uint32_t module_id) {
    if (!g_sandbox_registry_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        if (g_sandbox_registry[i] && g_sandbox_registry[i]->module_id == module_id) {
            return g_sandbox_registry[i];
        }
    }
    
    return NULL;
}

int idcu_sandbox_registry_check_perm(uint32_t module_id, uint32_t mask) {
    idcu_Sandbox* sb = idcu_sandbox_registry_get(module_id);
    if (!sb) {
        return IDCU_ERR_NOT_FOUND;
    }
    return idcu_sandbox_perm_check(sb, mask);
}
