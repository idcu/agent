#ifndef IDCU_MONITOR_SYSTEM_MONITOR_H
#define IDCU_MONITOR_SYSTEM_MONITOR_H

#include "error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_MONITOR_DISK_PATH_MAX 256
#define IDCU_MONITOR_PROCESS_NAME_MAX 256

typedef struct {
    double usage_percent;
    uint64_t total_cores;
    uint64_t active_cores;
    uint64_t user_time;
    uint64_t system_time;
    uint64_t idle_time;
} idcu_CPUStats;

typedef struct {
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
    uint64_t available_bytes;
    double usage_percent;
    uint64_t swap_total;
    uint64_t swap_used;
    uint64_t swap_free;
} idcu_MemoryStats;

typedef struct {
    char mount_point[IDCU_MONITOR_DISK_PATH_MAX];
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
    double usage_percent;
    uint64_t read_bytes;
    uint64_t write_bytes;
    uint64_t read_count;
    uint64_t write_count;
} idcu_DiskStats;

typedef struct {
    uint64_t process_id;
    char process_name[IDCU_MONITOR_PROCESS_NAME_MAX];
    double cpu_usage;
    uint64_t memory_usage;
    uint64_t uptime_ms;
    int threads;
    int is_running;
} idcu_ProcessStats;

typedef struct idcu_SystemMonitor idcu_SystemMonitor;

int idcu_system_monitor_init(idcu_SystemMonitor** monitor);
void idcu_system_monitor_destroy(idcu_SystemMonitor* monitor);

int idcu_system_monitor_get_cpu_stats(idcu_SystemMonitor* monitor, idcu_CPUStats* stats);
int idcu_system_monitor_get_memory_stats(idcu_SystemMonitor* monitor, idcu_MemoryStats* stats);
int idcu_system_monitor_get_disk_stats(idcu_SystemMonitor* monitor, idcu_DiskStats* stats_array, size_t* array_size);
int idcu_system_monitor_get_process_stats(idcu_SystemMonitor* monitor, uint64_t pid, idcu_ProcessStats* stats);
int idcu_system_monitor_get_current_process_stats(idcu_SystemMonitor* monitor, idcu_ProcessStats* stats);

uint64_t idcu_system_monitor_get_uptime_ms(idcu_SystemMonitor* monitor);
uint64_t idcu_system_monitor_get_timestamp_ms(idcu_SystemMonitor* monitor);

#endif // IDCU_MONITOR_SYSTEM_MONITOR_H
