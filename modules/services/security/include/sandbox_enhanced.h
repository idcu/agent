#ifndef IDCU_SECURITY_SANDBOX_ENHANCED_H
#define IDCU_SECURITY_SANDBOX_ENHANCED_H

#include "sandbox.h"
#include "idcu/common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_MAX_MEMORY_REGIONS 32
#define IDCU_MAX_SYSCALLS 64
#define IDCU_MAX_ENHANCED_SANDBOXES 64
#define IDCU_MEM_REGION_READ (1U << 0)
#define IDCU_MEM_REGION_WRITE (1U << 1)
#define IDCU_MEM_REGION_EXEC (1U << 2)
#define IDCU_MEM_REGION_ALLOCATED (1U << 3)

typedef struct {
    void* start_addr;
    size_t size;
    uint32_t permissions;
} idcu_MemoryRegion;

typedef struct {
    int syscall_num;
    char name[32];
} idcu_SyscallEntry;

typedef struct {
    idcu_Sandbox base;
    int active;
    idcu_MemoryRegion memory_regions[IDCU_MAX_MEMORY_REGIONS];
    int mem_region_count;
    idcu_SyscallEntry syscalls[IDCU_MAX_SYSCALLS];
    int syscall_count;
    uint64_t max_cpu_time_ms;
    uint64_t used_cpu_time_ms;
    uint64_t max_memory_bytes;
    uint64_t used_memory_bytes;
    uint32_t max_file_descriptors;
    uint32_t used_file_descriptors;
} idcu_EnhancedSandbox;

int idcu_enhanced_sandbox_init(idcu_EnhancedSandbox* sb, uint32_t module_id, uint32_t perm);
void idcu_enhanced_sandbox_destroy(idcu_EnhancedSandbox* sb);
int idcu_enhanced_sandbox_add_memory_region(idcu_EnhancedSandbox* sb, void* addr, size_t size, uint32_t perm);
int idcu_enhanced_sandbox_remove_memory_region(idcu_EnhancedSandbox* sb, void* addr);
int idcu_enhanced_sandbox_check_memory_access(idcu_EnhancedSandbox* sb, void* addr, size_t size, uint32_t required_perm);
int idcu_enhanced_sandbox_add_syscall(idcu_EnhancedSandbox* sb, int syscall_num, const char* name);
int idcu_enhanced_sandbox_remove_syscall(idcu_EnhancedSandbox* sb, int syscall_num);
int idcu_enhanced_sandbox_check_syscall(idcu_EnhancedSandbox* sb, int syscall_num);
int idcu_enhanced_sandbox_set_cpu_limit(idcu_EnhancedSandbox* sb, uint64_t limit_ms);
int idcu_enhanced_sandbox_set_memory_limit(idcu_EnhancedSandbox* sb, uint64_t limit_bytes);
int idcu_enhanced_sandbox_set_fd_limit(idcu_EnhancedSandbox* sb, uint32_t limit);
int idcu_enhanced_sandbox_check_cpu_usage(idcu_EnhancedSandbox* sb);
int idcu_enhanced_sandbox_check_fd_usage(idcu_EnhancedSandbox* sb, uint32_t requested_fds);
int idcu_enhanced_sandbox_update_memory_usage(idcu_EnhancedSandbox* sb, int64_t delta);
uint64_t idcu_enhanced_sandbox_get_memory_usage(idcu_EnhancedSandbox* sb);
void* idcu_enhanced_sandbox_malloc(idcu_EnhancedSandbox* sb, size_t size);
void idcu_enhanced_sandbox_free(idcu_EnhancedSandbox* sb, void* ptr);
void* idcu_enhanced_sandbox_calloc(idcu_EnhancedSandbox* sb, size_t nmemb, size_t size);
void* idcu_enhanced_sandbox_realloc(idcu_EnhancedSandbox* sb, void* ptr, size_t size);

int idcu_enhanced_sandbox_registry_init(void);
void idcu_enhanced_sandbox_registry_destroy(void);
int idcu_enhanced_sandbox_registry_add(idcu_EnhancedSandbox* sb);
int idcu_enhanced_sandbox_registry_remove(uint32_t module_id);
idcu_EnhancedSandbox* idcu_enhanced_sandbox_registry_get(uint32_t module_id);
int idcu_enhanced_sandbox_registry_check_memory_access(uint32_t module_id, void* addr, size_t size, uint32_t required_perm);

#define IDCU_SANDBOX_CHECK_READ(sb, addr, size) \
    idcu_enhanced_sandbox_check_memory_access((sb), (addr), (size), IDCU_MEM_REGION_READ)

#define IDCU_SANDBOX_CHECK_WRITE(sb, addr, size) \
    idcu_enhanced_sandbox_check_memory_access((sb), (addr), (size), IDCU_MEM_REGION_WRITE)

#define IDCU_SANDBOX_CHECK_READ_WRITE(sb, addr, size) \
    idcu_enhanced_sandbox_check_memory_access((sb), (addr), (size), IDCU_MEM_REGION_READ | IDCU_MEM_REGION_WRITE)

#endif // IDCU_SECURITY_SANDBOX_ENHANCED_H
