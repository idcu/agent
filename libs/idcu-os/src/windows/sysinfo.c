#include "idcu/os.h"
#include <windows.h>
#include <string.h>

int idcu_sysinfo_get(idcu_sysinfo_t* info)
{
    if (!info) {
        return -1;
    }
    MEMORYSTATUSEX memstat;
    memstat.dwLength = sizeof(memstat);
    if (!GlobalMemoryStatusEx(&memstat)) {
        return -1;
    }
    info->total_memory = memstat.ullTotalPhys;
    info->free_memory = memstat.ullAvailPhys;
    info->total_swap = memstat.ullTotalPageFile;
    info->free_swap = memstat.ullAvailPageFile;
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    info->cpu_count = sysinfo.dwNumberOfProcessors;
    strncpy(info->os_name, "Windows", sizeof(info->os_name) - 1);
    info->os_name[sizeof(info->os_name) - 1] = '\0';
    OSVERSIONINFOEXA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
    if (GetVersionExA((OSVERSIONINFOA*)&osvi)) {
        _snprintf(info->os_version, sizeof(info->os_version), "%lu.%lu.%lu",
                 osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    } else {
        strncpy(info->os_version, "Unknown", sizeof(info->os_version) - 1);
    }
    info->os_version[sizeof(info->os_version) - 1] = '\0';
    return 0;
}

int idcu_statvfs_get(const char* path, idcu_statvfs_t* statvfs_out)
{
    if (!path || !statvfs_out) {
        return -1;
    }
    ULARGE_INTEGER free_bytes, total_bytes, total_free_bytes;
    if (!GetDiskFreeSpaceExA(path, &free_bytes, &total_bytes, &total_free_bytes)) {
        return -1;
    }
    statvfs_out->block_size = 4096;
    statvfs_out->total_blocks = total_bytes.QuadPart / statvfs_out->block_size;
    statvfs_out->free_blocks = total_free_bytes.QuadPart / statvfs_out->block_size;
    statvfs_out->available_blocks = free_bytes.QuadPart / statvfs_out->block_size;
    statvfs_out->total_files = 0;
    statvfs_out->free_files = 0;
    return 0;
}
