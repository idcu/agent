#include "sandbox_enhanced.h"
#include <string.h>
#include <stdlib.h>

static idcu_EnhancedSandbox* g_enhanced_sandbox_registry[IDCU_MAX_ENHANCED_SANDBOXES] = {0};
static int g_enhanced_sandbox_registry_initialized = 0;

int idcu_enhanced_sandbox_init(idcu_EnhancedSandbox* sb, uint32_t module_id, uint32_t perm) {
    if (!sb) return IDCU_ERR_INVALID_PARAM;
    
    memset(sb, 0, sizeof(idcu_EnhancedSandbox));
    sb->base.module_id = module_id;
    sb->base.perm = perm;
    sb->active = 1;
    sb->mem_region_count = 0;
    sb->syscall_count = 0;
    sb->max_cpu_time_ms = 0;
    sb->used_cpu_time_ms = 0;
    sb->max_memory_bytes = 0;
    sb->used_memory_bytes = 0;
    sb->max_file_descriptors = 0;
    sb->used_file_descriptors = 0;
    
    return IDCU_ERR_OK;
}

void idcu_enhanced_sandbox_destroy(idcu_EnhancedSandbox* sb) {
    if (!sb) return;
    
    for (int i = 0; i < sb->mem_region_count; i++) {
        if (sb->memory_regions[i].permissions & IDCU_MEM_REGION_ALLOCATED) {
            free(sb->memory_regions[i].start_addr);
        }
    }
    
    memset(sb, 0, sizeof(idcu_EnhancedSandbox));
}

int idcu_enhanced_sandbox_add_memory_region(idcu_EnhancedSandbox* sb, void* addr, size_t size, uint32_t perm) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    if (sb->mem_region_count >= IDCU_MAX_MEMORY_REGIONS) return IDCU_ERR_NO_MEMORY;
    if (!addr || size == 0) return IDCU_ERR_INVALID_PARAM;
    
    sb->memory_regions[sb->mem_region_count].start_addr = addr;
    sb->memory_regions[sb->mem_region_count].size = size;
    sb->memory_regions[sb->mem_region_count].permissions = perm;
    sb->mem_region_count++;
    
    return IDCU_ERR_OK;
}

int idcu_enhanced_sandbox_remove_memory_region(idcu_EnhancedSandbox* sb, void* addr) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    if (!addr) return IDCU_ERR_INVALID_PARAM;
    
    for (int i = 0; i < sb->mem_region_count; i++) {
        if (sb->memory_regions[i].start_addr == addr) {
            if (sb->memory_regions[i].permissions & IDCU_MEM_REGION_ALLOCATED) {
                free(sb->memory_regions[i].start_addr);
            }
            for (int j = i; j < sb->mem_region_count - 1; j++) {
                sb->memory_regions[j] = sb->memory_regions[j + 1];
            }
            sb->mem_region_count--;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

int idcu_enhanced_sandbox_check_memory_access(idcu_EnhancedSandbox* sb, void* addr, size_t size, uint32_t required_perm) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    if (!addr || size == 0) return IDCU_ERR_INVALID_PARAM;
    
    uintptr_t start = (uintptr_t)addr;
    uintptr_t end = start + size;
    
    for (int i = 0; i < sb->mem_region_count; i++) {
        uintptr_t region_start = (uintptr_t)sb->memory_regions[i].start_addr;
        uintptr_t region_end = region_start + sb->memory_regions[i].size;
        
        if (start >= region_start && end <= region_end) {
            if ((sb->memory_regions[i].permissions & required_perm) == required_perm) {
                return IDCU_ERR_OK;
            } else {
                return IDCU_ERR_PERM_DENIED;
            }
        }
    }
    
    return IDCU_ERR_PERM_DENIED;
}

int idcu_enhanced_sandbox_add_syscall(idcu_EnhancedSandbox* sb, int syscall_num, const char* name) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    if (sb->syscall_count >= IDCU_MAX_SYSCALLS) return IDCU_ERR_NO_MEMORY;
    if (!name) return IDCU_ERR_INVALID_PARAM;
    
    sb->syscalls[sb->syscall_count].syscall_num = syscall_num;
    strncpy(sb->syscalls[sb->syscall_count].name, name, sizeof(sb->syscalls[sb->syscall_count].name) - 1);
    sb->syscalls[sb->syscall_count].name[sizeof(sb->syscalls[sb->syscall_count].name) - 1] = '\0';
    sb->syscall_count++;
    
    return IDCU_ERR_OK;
}

int idcu_enhanced_sandbox_remove_syscall(idcu_EnhancedSandbox* sb, int syscall_num) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    
    for (int i = 0; i < sb->syscall_count; i++) {
        if (sb->syscalls[i].syscall_num == syscall_num) {
            for (int j = i; j < sb->syscall_count - 1; j++) {
                sb->syscalls[j] = sb->syscalls[j + 1];
            }
            sb->syscall_count--;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

int idcu_enhanced_sandbox_check_syscall(idcu_EnhancedSandbox* sb, int syscall_num) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    
    for (int i = 0; i < sb->syscall_count; i++) {
        if (sb->syscalls[i].syscall_num == syscall_num) {
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_PERM_DENIED;
}

int idcu_enhanced_sandbox_set_cpu_limit(idcu_EnhancedSandbox* sb, uint64_t limit_ms) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    sb->max_cpu_time_ms = limit_ms;
    return IDCU_ERR_OK;
}

int idcu_enhanced_sandbox_set_memory_limit(idcu_EnhancedSandbox* sb, uint64_t limit_bytes) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    sb->max_memory_bytes = limit_bytes;
    return IDCU_ERR_OK;
}

int idcu_enhanced_sandbox_set_fd_limit(idcu_EnhancedSandbox* sb, uint32_t limit) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    sb->max_file_descriptors = limit;
    return IDCU_ERR_OK;
}

int idcu_enhanced_sandbox_check_cpu_usage(idcu_EnhancedSandbox* sb) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    if (sb->max_cpu_time_ms == 0) return IDCU_ERR_OK;
    if (sb->used_cpu_time_ms >= sb->max_cpu_time_ms) return IDCU_ERR_PERM_DENIED;
    return IDCU_ERR_OK;
}

int idcu_enhanced_sandbox_check_fd_usage(idcu_EnhancedSandbox* sb, uint32_t requested_fds) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    if (sb->max_file_descriptors == 0) return IDCU_ERR_OK;
    if (sb->used_file_descriptors + requested_fds > sb->max_file_descriptors) return IDCU_ERR_PERM_DENIED;
    return IDCU_ERR_OK;
}

int idcu_enhanced_sandbox_update_memory_usage(idcu_EnhancedSandbox* sb, int64_t delta) {
    if (!sb || !sb->active) return IDCU_ERR_INVALID_PARAM;
    
    if (delta < 0 && sb->used_memory_bytes < (uint64_t)(-delta)) {
        sb->used_memory_bytes = 0;
    } else {
        sb->used_memory_bytes += delta;
    }
    
    return IDCU_ERR_OK;
}

uint64_t idcu_enhanced_sandbox_get_memory_usage(idcu_EnhancedSandbox* sb) {
    if (!sb) return 0;
    return sb->used_memory_bytes;
}

void* idcu_enhanced_sandbox_malloc(idcu_EnhancedSandbox* sb, size_t size) {
    if (!sb || !sb->active) return NULL;
    if (size == 0) return NULL;
    
    if (sb->max_memory_bytes > 0 && 
        sb->used_memory_bytes + size > sb->max_memory_bytes) {
        return NULL;
    }
    
    void* ptr = malloc(size);
    if (!ptr) return NULL;
    
    int ret = idcu_enhanced_sandbox_add_memory_region(sb, ptr, size, 
        IDCU_MEM_REGION_READ | IDCU_MEM_REGION_WRITE | IDCU_MEM_REGION_ALLOCATED);
    if (ret != IDCU_ERR_OK) {
        free(ptr);
        return NULL;
    }
    
    idcu_enhanced_sandbox_update_memory_usage(sb, size);
    return ptr;
}

void idcu_enhanced_sandbox_free(idcu_EnhancedSandbox* sb, void* ptr) {
    if (!sb || !sb->active || !ptr) return;
    
    for (int i = 0; i < sb->mem_region_count; i++) {
        if (sb->memory_regions[i].start_addr == ptr) {
            size_t size = sb->memory_regions[i].size;
            idcu_enhanced_sandbox_remove_memory_region(sb, ptr);
            idcu_enhanced_sandbox_update_memory_usage(sb, -(int64_t)size);
            return;
        }
    }
}

void* idcu_enhanced_sandbox_calloc(idcu_EnhancedSandbox* sb, size_t nmemb, size_t size) {
    if (!sb || !sb->active) return NULL;
    if (nmemb == 0 || size == 0) return NULL;
    
    size_t total_size = nmemb * size;
    if (sb->max_memory_bytes > 0 && 
        sb->used_memory_bytes + total_size > sb->max_memory_bytes) {
        return NULL;
    }
    
    void* ptr = calloc(nmemb, size);
    if (!ptr) return NULL;
    
    int ret = idcu_enhanced_sandbox_add_memory_region(sb, ptr, total_size, 
        IDCU_MEM_REGION_READ | IDCU_MEM_REGION_WRITE | IDCU_MEM_REGION_ALLOCATED);
    if (ret != IDCU_ERR_OK) {
        free(ptr);
        return NULL;
    }
    
    idcu_enhanced_sandbox_update_memory_usage(sb, total_size);
    return ptr;
}

void* idcu_enhanced_sandbox_realloc(idcu_EnhancedSandbox* sb, void* ptr, size_t new_size) {
    if (!sb || !sb->active) return NULL;
    if (ptr == NULL) return idcu_enhanced_sandbox_malloc(sb, new_size);
    if (new_size == 0) {
        idcu_enhanced_sandbox_free(sb, ptr);
        return NULL;
    }
    
    int region_idx = -1;
    size_t old_size = 0;
    
    for (int i = 0; i < sb->mem_region_count; i++) {
        if (sb->memory_regions[i].start_addr == ptr) {
            region_idx = i;
            old_size = sb->memory_regions[i].size;
            break;
        }
    }
    
    if (region_idx == -1) return NULL;
    
    int64_t size_diff = (int64_t)new_size - (int64_t)old_size;
    if (sb->max_memory_bytes > 0 && size_diff > 0 &&
        sb->used_memory_bytes + size_diff > sb->max_memory_bytes) {
        return NULL;
    }
    
    void* new_ptr = realloc(ptr, new_size);
    if (!new_ptr) return NULL;
    
    sb->memory_regions[region_idx].start_addr = new_ptr;
    sb->memory_regions[region_idx].size = new_size;
    
    idcu_enhanced_sandbox_update_memory_usage(sb, size_diff);
    return new_ptr;
}

int idcu_enhanced_sandbox_registry_init(void) {
    if (g_enhanced_sandbox_registry_initialized) {
        return IDCU_ERR_OK;
    }
    
    memset(g_enhanced_sandbox_registry, 0, sizeof(g_enhanced_sandbox_registry));
    g_enhanced_sandbox_registry_initialized = 1;
    
    return IDCU_ERR_OK;
}

void idcu_enhanced_sandbox_registry_destroy(void) {
    if (!g_enhanced_sandbox_registry_initialized) {
        return;
    }
    
    for (int i = 0; i < IDCU_MAX_ENHANCED_SANDBOXES; i++) {
        if (g_enhanced_sandbox_registry[i]) {
            idcu_enhanced_sandbox_destroy(g_enhanced_sandbox_registry[i]);
            g_enhanced_sandbox_registry[i] = NULL;
        }
    }
    
    g_enhanced_sandbox_registry_initialized = 0;
}

int idcu_enhanced_sandbox_registry_add(idcu_EnhancedSandbox* sb) {
    if (!g_enhanced_sandbox_registry_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    if (!sb) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    for (int i = 0; i < IDCU_MAX_ENHANCED_SANDBOXES; i++) {
        if (g_enhanced_sandbox_registry[i] && 
            g_enhanced_sandbox_registry[i]->base.module_id == sb->base.module_id) {
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }
    
    for (int i = 0; i < IDCU_MAX_ENHANCED_SANDBOXES; i++) {
        if (!g_enhanced_sandbox_registry[i]) {
            g_enhanced_sandbox_registry[i] = sb;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NO_MEMORY;
}

int idcu_enhanced_sandbox_registry_remove(uint32_t module_id) {
    if (!g_enhanced_sandbox_registry_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }
    
    for (int i = 0; i < IDCU_MAX_ENHANCED_SANDBOXES; i++) {
        if (g_enhanced_sandbox_registry[i] && 
            g_enhanced_sandbox_registry[i]->base.module_id == module_id) {
            idcu_enhanced_sandbox_destroy(g_enhanced_sandbox_registry[i]);
            g_enhanced_sandbox_registry[i] = NULL;
            return IDCU_ERR_OK;
        }
    }
    
    return IDCU_ERR_NOT_FOUND;
}

idcu_EnhancedSandbox* idcu_enhanced_sandbox_registry_get(uint32_t module_id) {
    if (!g_enhanced_sandbox_registry_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < IDCU_MAX_ENHANCED_SANDBOXES; i++) {
        if (g_enhanced_sandbox_registry[i] && 
            g_enhanced_sandbox_registry[i]->base.module_id == module_id) {
            return g_enhanced_sandbox_registry[i];
        }
    }
    
    return NULL;
}

int idcu_enhanced_sandbox_registry_check_memory_access(uint32_t module_id, void* addr, size_t size, uint32_t required_perm) {
    idcu_EnhancedSandbox* sb = idcu_enhanced_sandbox_registry_get(module_id);
    if (!sb) {
        return IDCU_ERR_NOT_FOUND;
    }
    return idcu_enhanced_sandbox_check_memory_access(sb, addr, size, required_perm);
}
