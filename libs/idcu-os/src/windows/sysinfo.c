#include "idcu/os/os.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

int idcu_sysinfo_get(idcu_sysinfo_t* info)
{
    if (!info) {
        return -1;
    }
    
    memset(info, 0, sizeof(*info));
    
    MEMORYSTATUSEX memstat;
    memstat.dwLength = sizeof(memstat);
    if (GlobalMemoryStatusEx(&memstat)) {
        info->total_memory = memstat.ullTotalPhys;
        info->free_memory = memstat.ullAvailPhys;
        info->total_swap = memstat.ullTotalPageFile;
        info->free_swap = memstat.ullAvailPageFile;
    }
    
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    info->cpu_count = sysinfo.dwNumberOfProcessors;
    
    OSVERSIONINFOEXA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
    
    strcpy(info->os_name, "Windows");
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type, size;
        char product_name[64];
        size = sizeof(product_name);
        if (RegQueryValueExA(hKey, "ProductName", NULL, &type, (LPBYTE)product_name, &size) == ERROR_SUCCESS) {
            strncpy(info->os_name, product_name, sizeof(info->os_name) - 1);
        }
        
        char display_version[64];
        size = sizeof(display_version);
        if (RegQueryValueExA(hKey, "DisplayVersion", NULL, &type, (LPBYTE)display_version, &size) == ERROR_SUCCESS) {
            strncpy(info->os_version, display_version, sizeof(info->os_version) - 1);
        }
        RegCloseKey(hKey);
    }
    
    return 0;
}

int idcu_statvfs_get(const char* path, idcu_statvfs_t* statvfs)
{
    if (!path || !statvfs) {
        return -1;
    }
    
    memset(statvfs, 0, sizeof(*statvfs));
    
    ULARGE_INTEGER free_bytes, total_bytes, total_free_bytes;
    if (!GetDiskFreeSpaceExA(path, &free_bytes, &total_bytes, &total_free_bytes)) {
        return -1;
    }
    
    statvfs->block_size = 4096;
    statvfs->total_blocks = total_bytes.QuadPart / statvfs->block_size;
    statvfs->free_blocks = total_free_bytes.QuadPart / statvfs->block_size;
    statvfs->available_blocks = free_bytes.QuadPart / statvfs->block_size;
    
    return 0;
}
