#include "idcu/os/os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>

int idcu_sysinfo_get(idcu_sysinfo_t* info)
{
    if (!info) {
        return -1;
    }
    
    memset(info, 0, sizeof(*info));
    
    int mib[2];
    size_t size;
    u_int page_size;
    u_int64_t total_memory;
    u_int64_t free_memory;
    int cpu_count;
    
    mib[0] = CTL_HW;
    mib[1] = HW_PAGESIZE;
    size = sizeof(page_size);
    if (sysctl(mib, 2, &page_size, &size, NULL, 0) == 0) {
        mib[0] = CTL_HW;
        mib[1] = HW_PHYSMEM;
        size = sizeof(total_memory);
        if (sysctl(mib, 2, &total_memory, &size, NULL, 0) == 0) {
            info->total_memory = total_memory;
        }
        
        mib[0] = CTL_VM;
        mib[1] = VM_TOTAL;
        size = sizeof(free_memory);
        if (sysctl(mib, 2, &free_memory, &size, NULL, 0) == 0) {
            info->free_memory = free_memory * page_size;
        }
        
        mib[0] = CTL_HW;
        mib[1] = HW_NCPU;
        size = sizeof(cpu_count);
        if (sysctl(mib, 2, &cpu_count, &size, NULL, 0) == 0) {
            info->cpu_count = cpu_count;
        }
    }
    
    mib[0] = CTL_KERN;
    mib[1] = KERN_OSRELEASE;
    char os_release[256];
    size = sizeof(os_release);
    if (sysctl(mib, 2, os_release, &size, NULL, 0) == 0) {
        strncpy(info->os_version, os_release, sizeof(info->os_version) - 1);
    }
    
    strcpy(info->os_name, "FreeBSD");
    
    return 0;
}

int idcu_statvfs_get(const char* path, idcu_statvfs_t* statvfs)
{
    if (!path || !statvfs) {
        return -1;
    }
    
    memset(statvfs, 0, sizeof(*statvfs));
    
    struct statvfs sv;
    if (statvfs(path, &sv) != 0) {
        return -1;
    }
    
    statvfs->block_size = sv.f_bsize;
    statvfs->total_blocks = sv.f_blocks;
    statvfs->free_blocks = sv.f_bfree;
    statvfs->available_blocks = sv.f_bavail;
    statvfs->total_files = sv.f_files;
    statvfs->free_files = sv.f_ffree;
    
    return 0;
}
