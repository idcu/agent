#ifndef SANDBOX_ENHANCED_H
#define SANDBOX_ENHANCED_H

#include "sandbox.h"
#include "include/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define MAX_MEMORY_REGIONS 32
#define MAX_SYSCALLS 64
#define MEM_REGION_READ (1U << 0)
#define MEM_REGION_WRITE (1U << 1)
#define MEM_REGION_EXEC (1U << 2)

typedef struct {
    void* start_addr;
    size_t size;
    uint32_t permissions;
} MemoryRegion;

typedef struct {
    int syscall_num;
    char name[32];
} SyscallEntry;

typedef struct {
    Sandbox base;
    int active;
    MemoryRegion memory_regions[MAX_MEMORY_REGIONS];
    int mem_region_count;
    SyscallEntry syscalls[MAX_SYSCALLS];
    int syscall_count;
    uint64_t max_cpu_time_ms;
    uint64_t used_cpu_time_ms;
    uint64_t max_memory_bytes;
    uint64_t used_memory_bytes;
    uint32_t max_file_descriptors;
    uint32_t used_file_descriptors;
} EnhancedSandbox;

int enhanced_sandbox_init(EnhancedSandbox* sb, uint32_t module_id, uint32_t perm);
void enhanced_sandbox_destroy(EnhancedSandbox* sb);
int enhanced_sandbox_add_memory_region(EnhancedSandbox* sb, void* addr, size_t size, uint32_t perm);
int enhanced_sandbox_remove_memory_region(EnhancedSandbox* sb, void* addr);
int enhanced_sandbox_check_memory_access(EnhancedSandbox* sb, void* addr, size_t size, uint32_t required_perm);
int enhanced_sandbox_add_syscall(EnhancedSandbox* sb, int syscall_num, const char* name);
int enhanced_sandbox_remove_syscall(EnhancedSandbox* sb, int syscall_num);
int enhanced_sandbox_check_syscall(EnhancedSandbox* sb, int syscall_num);
int enhanced_sandbox_set_cpu_limit(EnhancedSandbox* sb, uint64_t limit_ms);
int enhanced_sandbox_set_memory_limit(EnhancedSandbox* sb, uint64_t limit_bytes);
int enhanced_sandbox_set_fd_limit(EnhancedSandbox* sb, uint32_t limit);
int enhanced_sandbox_check_cpu_usage(EnhancedSandbox* sb);
int enhanced_sandbox_check_fd_usage(EnhancedSandbox* sb, uint32_t requested_fds);

#endif
