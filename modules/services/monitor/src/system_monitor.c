#include "system_monitor.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <pwd.h>
#endif

struct idcu_SystemMonitor {
    uint64_t init_timestamp_ms;
    int initialized;
    uint64_t last_cpu_time;
    uint64_t last_cpu_idle;
};

static uint64_t get_current_timestamp_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int idcu_system_monitor_init(idcu_SystemMonitor** monitor) {
    if (!monitor) {
        return IDCU_ERR_INVALID_PARAM;
    }

    *monitor = (idcu_SystemMonitor*)malloc(sizeof(idcu_SystemMonitor));
    if (!*monitor) {
        return IDCU_ERR_NO_MEMORY;
    }

    memset(*monitor, 0, sizeof(idcu_SystemMonitor));
    (*monitor)->init_timestamp_ms = get_current_timestamp_ms();
    (*monitor)->last_cpu_time = 0;
    (*monitor)->last_cpu_idle = 0;
    (*monitor)->initialized = 1;

    IDCU_LOG_INFO("[system_monitor] Initialized");
    return IDCU_ERR_OK;
}

void idcu_system_monitor_destroy(idcu_SystemMonitor* monitor) {
    if (!monitor) {
        return;
    }

    free(monitor);
    IDCU_LOG_INFO("[system_monitor] Destroyed");
}

int idcu_system_monitor_get_cpu_stats(idcu_SystemMonitor* monitor, idcu_CPUStats* stats) {
    if (!monitor || !monitor->initialized || !stats) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(stats, 0, sizeof(idcu_CPUStats));

#ifdef _WIN32
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    stats->total_cores = sys_info.dwNumberOfProcessors;
    stats->active_cores = stats->total_cores;

    FILETIME idle_time, kernel_time, user_time;
    if (GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
        ULARGE_INTEGER ul_idle, ul_kernel, ul_user;
        ul_idle.LowPart = idle_time.dwLowDateTime;
        ul_idle.HighPart = idle_time.dwHighDateTime;
        ul_kernel.LowPart = kernel_time.dwLowDateTime;
        ul_kernel.HighPart = kernel_time.dwHighDateTime;
        ul_user.LowPart = user_time.dwLowDateTime;
        ul_user.HighPart = user_time.dwHighDateTime;

        stats->idle_time = ul_idle.QuadPart / 10000;
        stats->system_time = (ul_kernel.QuadPart - ul_idle.QuadPart) / 10000;
        stats->user_time = ul_user.QuadPart / 10000;

        uint64_t total_time = stats->idle_time + stats->system_time + stats->user_time;
        uint64_t total_delta = total_time - monitor->last_cpu_time;
        uint64_t idle_delta = stats->idle_time - monitor->last_cpu_idle;

        if (total_delta > 0) {
            stats->usage_percent = 100.0 * (1.0 - (double)idle_delta / (double)total_delta);
        }

        monitor->last_cpu_time = total_time;
        monitor->last_cpu_idle = stats->idle_time;
    }

#else
    stats->total_cores = sysconf(_SC_NPROCESSORS_ONLN);
    stats->active_cores = stats->total_cores;

    FILE* fp = fopen("/proc/stat", "r");
    if (fp) {
        char line[256];
        if (fgets(line, sizeof(line), fp)) {
            unsigned long long user, nice, system, idle, iowait, irq, softirq;
            if (sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu",
                      &user, &nice, &system, &idle, &iowait, &irq, &softirq) >= 4) {
                stats->user_time = user + nice;
                stats->system_time = system + irq + softirq;
                stats->idle_time = idle + iowait;

                uint64_t total_time = stats->user_time + stats->system_time + stats->idle_time;
                uint64_t total_delta = total_time - monitor->last_cpu_time;
                uint64_t idle_delta = stats->idle_time - monitor->last_cpu_idle;

                if (total_delta > 0) {
                    stats->usage_percent = 100.0 * (1.0 - (double)idle_delta / (double)total_delta);
                }

                monitor->last_cpu_time = total_time;
                monitor->last_cpu_idle = stats->idle_time;
            }
        }
        fclose(fp);
    }
#endif

    IDCU_LOG_DEBUG("[system_monitor] CPU: %.1f%%", stats->usage_percent);
    return IDCU_ERR_OK;
}

int idcu_system_monitor_get_memory_stats(idcu_SystemMonitor* monitor, idcu_MemoryStats* stats) {
    if (!monitor || !monitor->initialized || !stats) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(stats, 0, sizeof(idcu_MemoryStats));

#ifdef _WIN32
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    if (GlobalMemoryStatusEx(&mem_status)) {
        stats->total_bytes = mem_status.ullTotalPhys;
        stats->used_bytes = mem_status.ullTotalPhys - mem_status.ullAvailPhys;
        stats->free_bytes = mem_status.ullAvailPhys;
        stats->available_bytes = mem_status.ullAvailPhys;
        stats->usage_percent = mem_status.dwMemoryLoad;
        stats->swap_total = mem_status.ullTotalPageFile;
        stats->swap_used = mem_status.ullTotalPageFile - mem_status.ullAvailPageFile;
        stats->swap_free = mem_status.ullAvailPageFile;
    }

#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        stats->total_bytes = (uint64_t)info.totalram * info.mem_unit;
        stats->free_bytes = (uint64_t)info.freeram * info.mem_unit;
        stats->used_bytes = stats->total_bytes - stats->free_bytes;
        stats->available_bytes = stats->free_bytes;
        stats->usage_percent = 100.0 * (double)stats->used_bytes / (double)stats->total_bytes;
        stats->swap_total = (uint64_t)info.totalswap * info.mem_unit;
        stats->swap_free = (uint64_t)info.freeswap * info.mem_unit;
        stats->swap_used = stats->swap_total - stats->swap_free;
    }

    FILE* fp = fopen("/proc/meminfo", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            unsigned long long value;
            if (sscanf(line, "MemAvailable: %llu kB", &value) == 1) {
                stats->available_bytes = value * 1024;
            }
        }
        fclose(fp);
    }
#endif

    IDCU_LOG_DEBUG("[system_monitor] Memory: %.1f%% (%.2f GB / %.2f GB)",
                  stats->usage_percent,
                  (double)stats->used_bytes / (1024.0 * 1024.0 * 1024.0),
                  (double)stats->total_bytes / (1024.0 * 1024.0 * 1024.0));
    return IDCU_ERR_OK;
}

int idcu_system_monitor_get_disk_stats(idcu_SystemMonitor* monitor, idcu_DiskStats* stats_array, size_t* array_size) {
    if (!monitor || !monitor->initialized || !array_size) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (!stats_array) {
#ifdef _WIN32
        *array_size = 1;
#else
        *array_size = 1;
#endif
        return IDCU_ERR_OK;
    }

    size_t count = 0;
    size_t max_count = *array_size;

#ifdef _WIN32
    char drives[256];
    DWORD len = GetLogicalDriveStringsA(sizeof(drives), drives);
    if (len > 0 && len < sizeof(drives)) {
        char* drive = drives;
        while (*drive && count < max_count) {
            if (GetDriveTypeA(drive) == DRIVE_FIXED) {
                ULARGE_INTEGER free_bytes, total_bytes, free_bytes_avail;
                if (GetDiskFreeSpaceExA(drive, &free_bytes_avail, &total_bytes, &free_bytes)) {
                    strncpy(stats_array[count].mount_point, drive, IDCU_MONITOR_DISK_PATH_MAX - 1);
                    stats_array[count].mount_point[IDCU_MONITOR_DISK_PATH_MAX - 1] = '\0';
                    stats_array[count].total_bytes = total_bytes.QuadPart;
                    stats_array[count].free_bytes = free_bytes.QuadPart;
                    stats_array[count].used_bytes = total_bytes.QuadPart - free_bytes.QuadPart;
                    stats_array[count].usage_percent = 100.0 * (double)stats_array[count].used_bytes / (double)stats_array[count].total_bytes;
                    count++;
                }
            }
            drive += strlen(drive) + 1;
        }
    }

#else
    struct statvfs vfs;
    if (statvfs("/", &vfs) == 0 && count < max_count) {
        strncpy(stats_array[count].mount_point, "/", IDCU_MONITOR_DISK_PATH_MAX - 1);
        stats_array[count].mount_point[IDCU_MONITOR_DISK_PATH_MAX - 1] = '\0';
        stats_array[count].total_bytes = (uint64_t)vfs.f_blocks * vfs.f_frsize;
        stats_array[count].free_bytes = (uint64_t)vfs.f_bfree * vfs.f_frsize;
        stats_array[count].used_bytes = stats_array[count].total_bytes - stats_array[count].free_bytes;
        stats_array[count].usage_percent = 100.0 * (double)stats_array[count].used_bytes / (double)stats_array[count].total_bytes;
        count++;
    }
#endif

    *array_size = count;
    return IDCU_ERR_OK;
}

int idcu_system_monitor_get_process_stats(idcu_SystemMonitor* monitor, uint64_t pid, idcu_ProcessStats* stats) {
    if (!monitor || !monitor->initialized || !stats) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(stats, 0, sizeof(idcu_ProcessStats));
    stats->process_id = pid;
    stats->is_running = 1;

#ifdef _WIN32
    HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)pid);
    if (h_process) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(h_process, &pmc, sizeof(pmc))) {
            stats->memory_usage = pmc.WorkingSetSize;
        }

        FILETIME ft_create, ft_exit, ft_kernel, ft_user;
        if (GetProcessTimes(h_process, &ft_create, &ft_exit, &ft_kernel, &ft_user)) {
            ULARGE_INTEGER ul_create;
            ul_create.LowPart = ft_create.dwLowDateTime;
            ul_create.HighPart = ft_create.dwHighDateTime;
            uint64_t create_ms = (ul_create.QuadPart - 116444736000000000ULL) / 10000;
            stats->uptime_ms = get_current_timestamp_ms() - create_ms;
        }

        char module_name[MAX_PATH];
        if (GetModuleBaseNameA(h_process, NULL, module_name, sizeof(module_name))) {
            strncpy(stats->process_name, module_name, IDCU_MONITOR_PROCESS_NAME_MAX - 1);
            stats->process_name[IDCU_MONITOR_PROCESS_NAME_MAX - 1] = '\0';
        }

        CloseHandle(h_process);
    }

#else
    char path[256];
    snprintf(path, sizeof(path), "/proc/%llu/stat", (unsigned long long)pid);
    FILE* fp = fopen(path, "r");
    if (fp) {
        char comm[256];
        unsigned long utime, stime;
        unsigned long long starttime;
        int threads;
        if (fscanf(fp, "%*d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %d %*d %*d %*u %llu",
                  comm, &utime, &stime, &threads, &starttime) >= 5) {
            strncpy(stats->process_name, comm, IDCU_MONITOR_PROCESS_NAME_MAX - 1);
            stats->process_name[IDCU_MONITOR_PROCESS_NAME_MAX - 1] = '\0';
            stats->threads = threads;
            
            long clk_tck = sysconf(_SC_CLK_TCK);
            if (clk_tck > 0) {
                struct sysinfo info;
                if (sysinfo(&info) == 0) {
                    stats->uptime_ms = (uint64_t)(info.uptime - starttime / clk_tck) * 1000;
                }
            }
        }
        fclose(fp);
    }

    snprintf(path, sizeof(path), "/proc/%llu/statm", (unsigned long long)pid);
    fp = fopen(path, "r");
    if (fp) {
        unsigned long size;
        if (fscanf(fp, "%lu", &size) == 1) {
            long page_size = sysconf(_SC_PAGESIZE);
            stats->memory_usage = (uint64_t)size * page_size;
        }
        fclose(fp);
    }
#endif

    return IDCU_ERR_OK;
}

int idcu_system_monitor_get_current_process_stats(idcu_SystemMonitor* monitor, idcu_ProcessStats* stats) {
    if (!monitor || !monitor->initialized || !stats) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    return idcu_system_monitor_get_process_stats(monitor, GetCurrentProcessId(), stats);
#else
    return idcu_system_monitor_get_process_stats(monitor, getpid(), stats);
#endif
}

uint64_t idcu_system_monitor_get_uptime_ms(idcu_SystemMonitor* monitor) {
    if (!monitor || !monitor->initialized) {
        return 0;
    }
    return get_current_timestamp_ms() - monitor->init_timestamp_ms;
}

uint64_t idcu_system_monitor_get_timestamp_ms(idcu_SystemMonitor* monitor) {
    (void)monitor;
    return get_current_timestamp_ms();
}
