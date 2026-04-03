#include "security/sandbox_enhanced.h"
#include <string.h>

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
