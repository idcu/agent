#include "idcu/os/os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <unistd.h>

int idcu_sysinfo_get(idcu_sysinfo_t* info)
{
    if (!info) {
        return -1;
    }
    
    memset(info, 0, sizeof(*info));
    
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info->total_memory = si.totalram * si.mem_unit;
        info->free_memory = si.freeram * si.mem_unit;
        info->total_swap = si.totalswap * si.mem_unit;
        info->free_swap = si.freeswap * si.mem_unit;
        info->cpu_count = get_nprocs();
    }
    
    strcpy(info->os_name, "Vector OS");
    strcpy(info->os_version, "2.1");
    
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
