#ifndef IDCU_SERVER_MONITOR_H
#define IDCU_SERVER_MONITOR_H

#include "idcu/common/error_code.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IDCU_SERVER_MONITOR_NAME_MAX 256
#define IDCU_SERVER_MONITOR_DISK_MAX 16
#define IDCU_SERVER_MONITOR_NET_MAX  8

    typedef struct
    {
        char     device[64];
        char     mount_point[256];
        uint64_t total_bytes;
        uint64_t used_bytes;
        uint64_t free_bytes;
    } idcu_DiskInfo;

    typedef struct
    {
        char     interface_name[64];
        uint64_t rx_bytes;
        uint64_t tx_bytes;
        uint64_t rx_packets;
        uint64_t tx_packets;
    } idcu_NetworkInfo;

    typedef struct
    {
        double load_1min;
        double load_5min;
        double load_15min;
    } idcu_CPUInfo;

    typedef struct
    {
        uint64_t total_bytes;
        uint64_t used_bytes;
        uint64_t free_bytes;
        double   usage_percent;
    } idcu_MemoryInfo;

    typedef struct
    {
        char     hostname[IDCU_SERVER_MONITOR_NAME_MAX];
        char     os_name[64];
        char     os_version[64];
        char     kernel_version[64];
        char     uptime[64];
        uint64_t uptime_seconds;
    } idcu_SystemInfo;

    typedef struct
    {
        idcu_SystemInfo  system;
        idcu_CPUInfo     cpu;
        idcu_MemoryInfo  memory;
        idcu_DiskInfo    disks[IDCU_SERVER_MONITOR_DISK_MAX];
        int              disk_count;
        idcu_NetworkInfo networks[IDCU_SERVER_MONITOR_NET_MAX];
        int              network_count;
        uint64_t         timestamp_ms;
    } idcu_ServerMetrics;

    typedef struct
    {
        void* internal;
    } idcu_ServerMonitor;

    int  idcu_server_monitor_init(idcu_ServerMonitor* monitor);
    void idcu_server_monitor_destroy(idcu_ServerMonitor* monitor);

    int idcu_server_monitor_collect(idcu_ServerMonitor* monitor, idcu_ServerMetrics* metrics);
    int idcu_server_monitor_get_system_info(idcu_ServerMonitor* monitor, idcu_SystemInfo* info);
    int idcu_server_monitor_get_cpu_info(idcu_ServerMonitor* monitor, idcu_CPUInfo* info);
    int idcu_server_monitor_get_memory_info(idcu_ServerMonitor* monitor, idcu_MemoryInfo* info);
    int idcu_server_monitor_get_disk_info(idcu_ServerMonitor* monitor, idcu_DiskInfo* disks,
                                          int* count);
    int idcu_server_monitor_get_network_info(idcu_ServerMonitor* monitor,
                                             idcu_NetworkInfo* networks, int* count);

    int idcu_server_monitor_export_json(idcu_ServerMonitor* monitor, char* buffer,
                                        size_t buffer_size, size_t* output_size);
    int idcu_server_monitor_export_prometheus(idcu_ServerMonitor* monitor, char* buffer,
                                              size_t buffer_size, size_t* output_size);

#ifdef __cplusplus
}
#endif

#endif
