#include "idcu/sandbox/sandbox.h"
#include "idcu/log/log.h"
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
    sb->resources.memory_limit = IDCU_SANDBOX_DEFAULT_MEMORY_LIMIT;
    sb->resources.cpu_limit_percent = IDCU_SANDBOX_DEFAULT_CPU_LIMIT;
    sb->resources.network_limit_bytes = IDCU_SANDBOX_DEFAULT_NETWORK_LIMIT;
    
    IDCU_LOG_INFO("Sandbox initialized for module %u", module_id);
    
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

int idcu_sandbox_set_memory_limit(idcu_Sandbox* sb, uint64_t limit_bytes) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->resources.memory_limit = limit_bytes;
    IDCU_LOG_INFO("Sandbox memory limit set to %llu bytes for module %u", 
                  (unsigned long long)limit_bytes, sb->module_id);
    return IDCU_ERR_OK;
}

int idcu_sandbox_set_cpu_limit(idcu_Sandbox* sb, uint32_t limit_percent) {
    if (!sb || limit_percent > 100) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->resources.cpu_limit_percent = limit_percent;
    IDCU_LOG_INFO("Sandbox CPU limit set to %u%% for module %u", 
                  limit_percent, sb->module_id);
    return IDCU_ERR_OK;
}

int idcu_sandbox_set_network_limit(idcu_Sandbox* sb, uint64_t limit_bytes) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->resources.network_limit_bytes = limit_bytes;
    IDCU_LOG_INFO("Sandbox network limit set to %llu bytes for module %u", 
                  (unsigned long long)limit_bytes, sb->module_id);
    return IDCU_ERR_OK;
}

int idcu_sandbox_check_memory(idcu_Sandbox* sb, uint64_t requested_bytes) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (sb->resources.memory_used + requested_bytes > sb->resources.memory_limit) {
        IDCU_LOG_ERROR("Sandbox memory limit exceeded for module %u: used=%llu, requested=%llu, limit=%llu",
                      sb->module_id, (unsigned long long)sb->resources.memory_used,
                      (unsigned long long)requested_bytes, (unsigned long long)sb->resources.memory_limit);
        return IDCU_ERR_RESOURCE_EXHAUSTED;
    }
    return IDCU_ERR_OK;
}

int idcu_sandbox_check_cpu(idcu_Sandbox* sb) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (sb->resources.cpu_usage_percent > sb->resources.cpu_limit_percent) {
        IDCU_LOG_ERROR("Sandbox CPU limit exceeded for module %u: usage=%u%%, limit=%u%%",
                      sb->module_id, sb->resources.cpu_usage_percent, sb->resources.cpu_limit_percent);
        return IDCU_ERR_RESOURCE_EXHAUSTED;
    }
    return IDCU_ERR_OK;
}

int idcu_sandbox_check_network(idcu_Sandbox* sb, uint64_t requested_bytes) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    uint64_t total_network = sb->resources.network_rx_bytes + sb->resources.network_tx_bytes;
    if (total_network + requested_bytes > sb->resources.network_limit_bytes) {
        IDCU_LOG_ERROR("Sandbox network limit exceeded for module %u: total=%llu, requested=%llu, limit=%llu",
                      sb->module_id, (unsigned long long)total_network,
                      (unsigned long long)requested_bytes, (unsigned long long)sb->resources.network_limit_bytes);
        return IDCU_ERR_RESOURCE_EXHAUSTED;
    }
    return IDCU_ERR_OK;
}

int idcu_sandbox_update_memory_usage(idcu_Sandbox* sb, int64_t delta_bytes) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int64_t new_usage = (int64_t)sb->resources.memory_used + delta_bytes;
    if (new_usage < 0) {
        new_usage = 0;
    }
    sb->resources.memory_used = (uint64_t)new_usage;
    return IDCU_ERR_OK;
}

int idcu_sandbox_update_network_rx(idcu_Sandbox* sb, uint64_t bytes) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->resources.network_rx_bytes += bytes;
    return IDCU_ERR_OK;
}

int idcu_sandbox_update_network_tx(idcu_Sandbox* sb, uint64_t bytes) {
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    sb->resources.network_tx_bytes += bytes;
    return IDCU_ERR_OK;
}

int idcu_sandbox_get_resource_usage(idcu_Sandbox* sb, idcu_ResourceUsage* usage) {
    if (!sb || !usage) {
        return IDCU_ERR_INVALID_PARAM;
    }
    memcpy(usage, &sb->resources, sizeof(idcu_ResourceUsage));
    return IDCU_ERR_OK;
}

int idcu_sandbox_registry_check_memory(uint32_t module_id, uint64_t requested_bytes) {
    idcu_Sandbox* sb = idcu_sandbox_registry_get(module_id);
    if (!sb) {
        return IDCU_ERR_NOT_FOUND;
    }
    return idcu_sandbox_check_memory(sb, requested_bytes);
}

int idcu_sandbox_registry_check_network(uint32_t module_id, uint64_t requested_bytes) {
    idcu_Sandbox* sb = idcu_sandbox_registry_get(module_id);
    if (!sb) {
        return IDCU_ERR_NOT_FOUND;
    }
    return idcu_sandbox_check_network(sb, requested_bytes);
}
