#include "idcu/server_monitor/server_monitor.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#else
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

struct idcu_ServerMonitorInternal {
    idcu_SystemInfo cached_system;
    int system_cached;
};

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int idcu_server_monitor_init(idcu_ServerMonitor* monitor) {
    if (!monitor) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_ServerMonitorInternal* internal = 
        (struct idcu_ServerMonitorInternal*)malloc(sizeof(struct idcu_ServerMonitorInternal));
    if (!internal) return IDCU_ERR_NO_MEMORY;
    
    memset(internal, 0, sizeof(struct idcu_ServerMonitorInternal));
    monitor->internal = internal;
    
    return IDCU_ERR_OK;
}

void idcu_server_monitor_destroy(idcu_ServerMonitor* monitor) {
    if (!monitor || !monitor->internal) return;
    free(monitor->internal);
    monitor->internal = NULL;
}

int idcu_server_monitor_get_system_info(idcu_ServerMonitor* monitor, idcu_SystemInfo* info) {
    if (!monitor || !info) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_ServerMonitorInternal* internal = 
        (struct idcu_ServerMonitorInternal*)monitor->internal;
    
    if (internal->system_cached) {
        *info = internal->cached_system;
        return IDCU_ERR_OK;
    }
    
    memset(info, 0, sizeof(idcu_SystemInfo));
    
#ifdef _WIN32
    DWORD size = sizeof(info->hostname);
    GetComputerNameA(info->hostname, &size);
    strncpy(info->os_name, "Windows", sizeof(info->os_name) - 1);
    
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        snprintf(info->os_version, sizeof(info->os_version), 
                "%lu.%lu.%lu", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    }
    
    ULONGLONG uptime = GetTickCount64() / 1000;
    info->uptime_seconds = (uint64_t)uptime;
    snprintf(info->uptime, sizeof(info->uptime), 
            "%llu days, %llu hours, %llu minutes", 
            uptime / 86400, (uptime % 86400) / 3600, (uptime % 3600) / 60);
#else
    struct utsname uts;
    if (uname(&uts) == 0) {
        strncpy(info->hostname, uts.nodename, sizeof(info->hostname) - 1);
        strncpy(info->os_name, uts.sysname, sizeof(info->os_name) - 1);
        strncpy(info->os_version, uts.release, sizeof(info->os_version) - 1);
        strncpy(info->kernel_version, uts.version, sizeof(info->kernel_version) - 1);
    }
    
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info->uptime_seconds = (uint64_t)si.uptime;
        snprintf(info->uptime, sizeof(info->uptime), 
                "%lu days, %lu hours, %lu minutes", 
                si.uptime / 86400, (si.uptime % 86400) / 3600, (si.uptime % 3600) / 60);
    }
#endif
    
    internal->cached_system = *info;
    internal->system_cached = 1;
    
    return IDCU_ERR_OK;
}

int idcu_server_monitor_get_cpu_info(idcu_ServerMonitor* monitor, idcu_CPUInfo* info) {
    if (!monitor || !info) return IDCU_ERR_INVALID_PARAM;
    
    memset(info, 0, sizeof(idcu_CPUInfo));
    
#ifdef _WIN32
    info->load_1min = 0.0;
    info->load_5min = 0.0;
    info->load_15min = 0.0;
#else
    double loadavg[3];
    if (getloadavg(loadavg, 3) == 3) {
        info->load_1min = loadavg[0];
        info->load_5min = loadavg[1];
        info->load_15min = loadavg[2];
    }
#endif
    
    return IDCU_ERR_OK;
}

int idcu_server_monitor_get_memory_info(idcu_ServerMonitor* monitor, idcu_MemoryInfo* info) {
    if (!monitor || !info) return IDCU_ERR_INVALID_PARAM;
    
    memset(info, 0, sizeof(idcu_MemoryInfo));
    
#ifdef _WIN32
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memStatus)) {
        info->total_bytes = memStatus.ullTotalPhys;
        info->used_bytes = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
        info->free_bytes = memStatus.ullAvailPhys;
        info->usage_percent = memStatus.dwMemoryLoad;
    }
#else
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info->total_bytes = (uint64_t)si.totalram * si.mem_unit;
        info->free_bytes = (uint64_t)si.freeram * si.mem_unit;
        info->used_bytes = info->total_bytes - info->free_bytes;
        info->usage_percent = 100.0 * info->used_bytes / info->total_bytes;
    }
#endif
    
    return IDCU_ERR_OK;
}

int idcu_server_monitor_get_disk_info(idcu_ServerMonitor* monitor, idcu_DiskInfo* disks, int* count) {
    if (!monitor || !disks || !count) return IDCU_ERR_INVALID_PARAM;
    
    int max_count = *count;
    *count = 0;
    
#ifdef _WIN32
    char drives[256];
    DWORD size = GetLogicalDriveStringsA(sizeof(drives), drives);
    if (size > 0 && size <= sizeof(drives)) {
        char* drive = drives;
        while (*drive && *count < max_count) {
            UINT driveType = GetDriveTypeA(drive);
            if (driveType == DRIVE_FIXED) {
                strncpy(disks[*count].device, drive, sizeof(disks[*count].device) - 1);
                strncpy(disks[*count].mount_point, drive, sizeof(disks[*count].mount_point) - 1);
                
                ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
                if (GetDiskFreeSpaceExA(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
                    disks[*count].total_bytes = totalBytes.QuadPart;
                    disks[*count].used_bytes = totalBytes.QuadPart - totalFreeBytes.QuadPart;
                    disks[*count].free_bytes = totalFreeBytes.QuadPart;
                }
                (*count)++;
            }
            drive += strlen(drive) + 1;
        }
    }
#else
    FILE* mounts = fopen("/proc/mounts", "r");
    if (mounts) {
        char line[512];
        while (fgets(line, sizeof(line), mounts) && *count < max_count) {
            char device[128], mount[256], fs[64];
            if (sscanf(line, "%127s %255s %63s", device, mount, fs) == 3) {
                if (strcmp(fs, "ext4") == 0 || strcmp(fs, "xfs") == 0 || 
                    strcmp(fs, "btrfs") == 0 || strcmp(fs, "vfat") == 0) {
                    strncpy(disks[*count].device, device, sizeof(disks[*count].device) - 1);
                    strncpy(disks[*count].mount_point, mount, sizeof(disks[*count].mount_point) - 1);
                    
                    struct statvfs stat;
                    if (statvfs(mount, &stat) == 0) {
                        disks[*count].total_bytes = (uint64_t)stat.f_blocks * stat.f_frsize;
                        disks[*count].free_bytes = (uint64_t)stat.f_bfree * stat.f_frsize;
                        disks[*count].used_bytes = disks[*count].total_bytes - disks[*count].free_bytes;
                    }
                    (*count)++;
                }
            }
        }
        fclose(mounts);
    }
#endif
    
    return IDCU_ERR_OK;
}

int idcu_server_monitor_get_network_info(idcu_ServerMonitor* monitor, idcu_NetworkInfo* networks, int* count) {
    if (!monitor || !networks || !count) return IDCU_ERR_INVALID_PARAM;
    
    int max_count = *count;
    *count = 0;
    
    return IDCU_ERR_OK;
}

int idcu_server_monitor_collect(idcu_ServerMonitor* monitor, idcu_ServerMetrics* metrics) {
    if (!monitor || !metrics) return IDCU_ERR_INVALID_PARAM;
    
    memset(metrics, 0, sizeof(idcu_ServerMetrics));
    metrics->timestamp_ms = get_current_time_ms();
    
    idcu_server_monitor_get_system_info(monitor, &metrics->system);
    idcu_server_monitor_get_cpu_info(monitor, &metrics->cpu);
    idcu_server_monitor_get_memory_info(monitor, &metrics->memory);
    
    int disk_count = IDCU_SERVER_MONITOR_DISK_MAX;
    idcu_server_monitor_get_disk_info(monitor, metrics->disks, &disk_count);
    metrics->disk_count = disk_count;
    
    int net_count = IDCU_SERVER_MONITOR_NET_MAX;
    idcu_server_monitor_get_network_info(monitor, metrics->networks, &net_count);
    metrics->network_count = net_count;
    
    return IDCU_ERR_OK;
}

int idcu_server_monitor_export_json(idcu_ServerMonitor* monitor, char* buffer, size_t buffer_size, size_t* output_size) {
    if (!monitor || !buffer || !output_size) return IDCU_ERR_INVALID_PARAM;
    
    idcu_ServerMetrics metrics;
    int ret = idcu_server_monitor_collect(monitor, &metrics);
    if (ret != IDCU_ERR_OK) return ret;
    
    int written = snprintf(buffer, buffer_size, 
        "{"
        "\"timestamp\":%llu,"
        "\"system\":{"
            "\"hostname\":\"%s\","
            "\"os_name\":\"%s\","
            "\"os_version\":\"%s\","
            "\"kernel_version\":\"%s\","
            "\"uptime\":\"%s\","
            "\"uptime_seconds\":%llu"
        "},"
        "\"cpu\":{"
            "\"load_1min\":%.2f,"
            "\"load_5min\":%.2f,"
            "\"load_15min\":%.2f"
        "},"
        "\"memory\":{"
            "\"total_bytes\":%llu,"
            "\"used_bytes\":%llu,"
            "\"free_bytes\":%llu,"
            "\"usage_percent\":%.2f"
        "},"
        "\"disks\":[",
        (unsigned long long)metrics.timestamp_ms,
        metrics.system.hostname,
        metrics.system.os_name,
        metrics.system.os_version,
        metrics.system.kernel_version,
        metrics.system.uptime,
        (unsigned long long)metrics.system.uptime_seconds,
        metrics.cpu.load_1min,
        metrics.cpu.load_5min,
        metrics.cpu.load_15min,
        (unsigned long long)metrics.memory.total_bytes,
        (unsigned long long)metrics.memory.used_bytes,
        (unsigned long long)metrics.memory.free_bytes,
        metrics.memory.usage_percent
    );
    
    if (written < 0 || (size_t)written >= buffer_size) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    for (int i = 0; i < metrics.disk_count; i++) {
        if (i > 0) {
            written += snprintf(buffer + written, buffer_size - written, ",");
        }
        int disk_written = snprintf(buffer + written, buffer_size - written,
            "{"
                "\"device\":\"%s\","
                "\"mount_point\":\"%s\","
                "\"total_bytes\":%llu,"
                "\"used_bytes\":%llu,"
                "\"free_bytes\":%llu"
            "}",
            metrics.disks[i].device,
            metrics.disks[i].mount_point,
            (unsigned long long)metrics.disks[i].total_bytes,
            (unsigned long long)metrics.disks[i].used_bytes,
            (unsigned long long)metrics.disks[i].free_bytes
        );
        if (disk_written < 0 || (size_t)(written + disk_written) >= buffer_size) {
            return IDCU_ERR_NO_MEMORY;
        }
        written += disk_written;
    }
    
    written += snprintf(buffer + written, buffer_size - written, "]}");
    if (written < 0 || (size_t)written >= buffer_size) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    *output_size = written;
    return IDCU_ERR_OK;
}

int idcu_server_monitor_export_prometheus(idcu_ServerMonitor* monitor, char* buffer, size_t buffer_size, size_t* output_size) {
    if (!monitor || !buffer || !output_size) return IDCU_ERR_INVALID_PARAM;
    
    idcu_ServerMetrics metrics;
    int ret = idcu_server_monitor_collect(monitor, &metrics);
    if (ret != IDCU_ERR_OK) return ret;
    
    int written = 0;
    
    written += snprintf(buffer + written, buffer_size - written,
        "# HELP idcu_server_memory_total_bytes Total memory in bytes\n"
        "# TYPE idcu_server_memory_total_bytes gauge\n"
        "idcu_server_memory_total_bytes %llu\n"
        "# HELP idcu_server_memory_used_bytes Used memory in bytes\n"
        "# TYPE idcu_server_memory_used_bytes gauge\n"
        "idcu_server_memory_used_bytes %llu\n"
        "# HELP idcu_server_memory_free_bytes Free memory in bytes\n"
        "# TYPE idcu_server_memory_free_bytes gauge\n"
        "idcu_server_memory_free_bytes %llu\n"
        "# HELP idcu_server_memory_usage_percent Memory usage percentage\n"
        "# TYPE idcu_server_memory_usage_percent gauge\n"
        "idcu_server_memory_usage_percent %.2f\n",
        (unsigned long long)metrics.memory.total_bytes,
        (unsigned long long)metrics.memory.used_bytes,
        (unsigned long long)metrics.memory.free_bytes,
        metrics.memory.usage_percent
    );
    
    written += snprintf(buffer + written, buffer_size - written,
        "# HELP idcu_server_cpu_load_1min CPU load average 1 minute\n"
        "# TYPE idcu_server_cpu_load_1min gauge\n"
        "idcu_server_cpu_load_1min %.2f\n"
        "# HELP idcu_server_cpu_load_5min CPU load average 5 minutes\n"
        "# TYPE idcu_server_cpu_load_5min gauge\n"
        "idcu_server_cpu_load_5min %.2f\n"
        "# HELP idcu_server_cpu_load_15min CPU load average 15 minutes\n"
        "# TYPE idcu_server_cpu_load_15min gauge\n"
        "idcu_server_cpu_load_15min %.2f\n",
        metrics.cpu.load_1min,
        metrics.cpu.load_5min,
        metrics.cpu.load_15min
    );
    
    for (int i = 0; i < metrics.disk_count; i++) {
        written += snprintf(buffer + written, buffer_size - written,
            "# HELP idcu_server_disk_total_bytes Total disk space in bytes\n"
            "# TYPE idcu_server_disk_total_bytes gauge\n"
            "idcu_server_disk_total_bytes{device=\"%s\",mount=\"%s\"} %llu\n"
            "# HELP idcu_server_disk_used_bytes Used disk space in bytes\n"
            "# TYPE idcu_server_disk_used_bytes gauge\n"
            "idcu_server_disk_used_bytes{device=\"%s\",mount=\"%s\"} %llu\n"
            "# HELP idcu_server_disk_free_bytes Free disk space in bytes\n"
            "# TYPE idcu_server_disk_free_bytes gauge\n"
            "idcu_server_disk_free_bytes{device=\"%s\",mount=\"%s\"} %llu\n",
            metrics.disks[i].device, metrics.disks[i].mount_point,
            (unsigned long long)metrics.disks[i].total_bytes,
            metrics.disks[i].device, metrics.disks[i].mount_point,
            (unsigned long long)metrics.disks[i].used_bytes,
            metrics.disks[i].device, metrics.disks[i].mount_point,
            (unsigned long long)metrics.disks[i].free_bytes
        );
    }
    
    if (written < 0 || (size_t)written >= buffer_size) {
        return IDCU_ERR_NO_MEMORY;
    }
    
    *output_size = written;
    return IDCU_ERR_OK;
}
