#include "idcu/os.h"
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <string.h>

int idcu_sysinfo_get(idcu_sysinfo_t* info)
{
    if (!info) {
        return -1;
    }
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        return -1;
    }
    info->total_memory = si.totalram * si.mem_unit;
    info->free_memory = si.freeram * si.mem_unit;
    info->total_swap = si.totalswap * si.mem_unit;
    info->free_swap = si.freeswap * si.mem_unit;
    info->cpu_count = get_nprocs();
    strncpy(info->os_name, "Linux", sizeof(info->os_name) - 1);
    info->os_name[sizeof(info->os_name) - 1] = '\0';
    strncpy(info->os_version, "Unknown", sizeof(info->os_version) - 1);
    info->os_version[sizeof(info->os_version) - 1] = '\0';
    return 0;
}

int idcu_statvfs_get(const char* path, idcu_statvfs_t* statvfs_out)
{
    if (!path || !statvfs_out) {
        return -1;
    }
    struct statvfs sv;
    if (statvfs(path, &sv) != 0) {
        return -1;
    }
    statvfs_out->block_size = sv.f_bsize;
    statvfs_out->total_blocks = sv.f_blocks;
    statvfs_out->free_blocks = sv.f_bfree;
    statvfs_out->available_blocks = sv.f_bavail;
    statvfs_out->total_files = sv.f_files;
    statvfs_out->free_files = sv.f_ffree;
    return 0;
}
