#include "idcu/os/os.h"
#include <rtthread.h>
#include <string.h>

int idcu_sysinfo_get(idcu_sysinfo_t* info)
{
    if (!info) {
        return -1;
    }
    memset(info, 0, sizeof(*info));
    
    strncpy(info->os_name, "RT-Thread", sizeof(info->os_name) - 1);
    strncpy(info->os_version, RT_VERSION, sizeof(info->os_version) - 1);
    
    info->cpu_count = 1;
    info->total_memory = (uint64_t)rt_total_memory_get();
    info->free_memory = (uint64_t)rt_memory_available();
    
    return 0;
}

int idcu_statvfs_get(const char* path, idcu_statvfs_t* buf)
{
    if (!buf) {
        return -1;
    }
    memset(buf, 0, sizeof(*buf));
    
    buf->f_bsize = 4096;
    buf->f_frsize = 4096;
    buf->f_blocks = 0;
    buf->f_bfree = 0;
    buf->f_bavail = 0;
    buf->f_files = 0;
    buf->f_ffree = 0;
    buf->f_favail = 0;
    buf->f_flag = 0;
    buf->f_namemax = 255;
    
    return 0;
}
