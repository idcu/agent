#include <idcu/common/error_code.h>
#include <idcu/server_monitor/server_monitor.h>
#include <stdio.h>

int main(void) {
    printf("=== idcu-server-monitor Basic Example ===\n\n");

    int ret;
    idcu_ServerMonitor monitor;

    printf("1. Initialize Server Monitor\n");
    printf("-------------------------------\n");
    ret = idcu_server_monitor_init(&monitor);
    if (ret != IDCU_ERR_OK) {
        printf("Server monitor init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("Server monitor initialized\n\n");

    printf("2. Get System Info\n");
    printf("--------------------\n");
    idcu_SystemInfo system_info;
    ret = idcu_server_monitor_get_system_info(&monitor, &system_info);
    if (ret == IDCU_ERR_OK) {
        printf("Hostname: %s\n", system_info.hostname);
        printf("OS: %s %s\n", system_info.os_name, system_info.os_version);
        printf("Kernel: %s\n", system_info.kernel_version);
        printf("Uptime: %s (%llu seconds)\n", system_info.uptime,
               (unsigned long long)system_info.uptime_seconds);
    }
    printf("\n");

    printf("3. Get CPU Info\n");
    printf("-----------------\n");
    idcu_CPUInfo cpu_info;
    ret = idcu_server_monitor_get_cpu_info(&monitor, &cpu_info);
    if (ret == IDCU_ERR_OK) {
        printf("Load 1min: %.2f\n", cpu_info.load_1min);
        printf("Load 5min: %.2f\n", cpu_info.load_5min);
        printf("Load 15min: %.2f\n", cpu_info.load_15min);
    }
    printf("\n");

    printf("4. Get Memory Info\n");
    printf("--------------------\n");
    idcu_MemoryInfo memory_info;
    ret = idcu_server_monitor_get_memory_info(&monitor, &memory_info);
    if (ret == IDCU_ERR_OK) {
        printf("Total: %llu MB\n", (unsigned long long)(memory_info.total_bytes / 1024 / 1024));
        printf("Used: %llu MB\n", (unsigned long long)(memory_info.used_bytes / 1024 / 1024));
        printf("Free: %llu MB\n", (unsigned long long)(memory_info.free_bytes / 1024 / 1024));
        printf("Usage: %.1f%%\n", memory_info.usage_percent);
    }
    printf("\n");

    printf("5. Get Disk Info\n");
    printf("------------------\n");
    idcu_DiskInfo disks[IDCU_SERVER_MONITOR_DISK_MAX];
    int disk_count = IDCU_SERVER_MONITOR_DISK_MAX;
    ret = idcu_server_monitor_get_disk_info(&monitor, disks, &disk_count);
    if (ret == IDCU_ERR_OK) {
        printf("Found %d disks:\n", disk_count);
        for (int i = 0; i < disk_count; i++) {
            printf("  - %s (%s)\n", disks[i].device, disks[i].mount_point);
            printf("    Total: %llu MB\n",
                   (unsigned long long)(disks[i].total_bytes / 1024 / 1024));
            printf("    Used: %llu MB\n", (unsigned long long)(disks[i].used_bytes / 1024 / 1024));
            printf("    Free: %llu MB\n", (unsigned long long)(disks[i].free_bytes / 1024 / 1024));
        }
    }
    printf("\n");

    printf("6. Get Network Info\n");
    printf("---------------------\n");
    idcu_NetworkInfo networks[IDCU_SERVER_MONITOR_NET_MAX];
    int network_count = IDCU_SERVER_MONITOR_NET_MAX;
    ret = idcu_server_monitor_get_network_info(&monitor, networks, &network_count);
    if (ret == IDCU_ERR_OK) {
        printf("Found %d network interfaces:\n", network_count);
        for (int i = 0; i < network_count; i++) {
            printf("  - %s\n", networks[i].interface_name);
            printf("    RX: %llu bytes / %llu packets\n", (unsigned long long)networks[i].rx_bytes,
                   (unsigned long long)networks[i].rx_packets);
            printf("    TX: %llu bytes / %llu packets\n", (unsigned long long)networks[i].tx_bytes,
                   (unsigned long long)networks[i].tx_packets);
        }
    }
    printf("\n");

    printf("7. Collect All Metrics\n");
    printf("------------------------\n");
    idcu_ServerMetrics metrics;
    ret = idcu_server_monitor_collect(&monitor, &metrics);
    if (ret == IDCU_ERR_OK) {
        printf("Metrics collected at timestamp: %llu\n", (unsigned long long)metrics.timestamp_ms);
        printf("System: %s\n", metrics.system.hostname);
        printf("CPU Load: %.2f\n", metrics.cpu.load_1min);
        printf("Memory: %.1f%% used\n", metrics.memory.usage_percent);
    }
    printf("\n");

    printf("8. Destroy Server Monitor\n");
    printf("---------------------------\n");
    idcu_server_monitor_destroy(&monitor);
    printf("Server monitor destroyed\n\n");

    printf("=== Example Complete ===\n");
    return 0;
}
