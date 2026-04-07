#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "idcu/server_monitor/server_monitor.h"
#include "idcu/log/log.h"

static void print_usage(const char* prog) {
    printf("Usage: %s <metric>\n", prog);
    printf("Available metrics:\n");
    printf("  cpu.load.1min       - CPU load average 1 minute\n");
    printf("  cpu.load.5min       - CPU load average 5 minutes\n");
    printf("  cpu.load.15min      - CPU load average 15 minutes\n");
    printf("  memory.total        - Total memory in bytes\n");
    printf("  memory.used         - Used memory in bytes\n");
    printf("  memory.free         - Free memory in bytes\n");
    printf("  memory.usage        - Memory usage percentage\n");
    printf("  system.hostname     - System hostname\n");
    printf("  system.uptime       - System uptime in seconds\n");
    printf("  discover            - Discover all disk metrics\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    idcu_log_init();
    
    idcu_ServerMonitor monitor;
    if (idcu_server_monitor_init(&monitor) != IDCU_ERR_OK) {
        printf("ZBX_NOTSUPPORTED\n");
        return 1;
    }
    
    idcu_ServerMetrics metrics;
    if (idcu_server_monitor_collect(&monitor, &metrics) != IDCU_ERR_OK) {
        printf("ZBX_NOTSUPPORTED\n");
        idcu_server_monitor_destroy(&monitor);
        return 1;
    }
    
    const char* metric = argv[1];
    
    if (strcmp(metric, "cpu.load.1min") == 0) {
        printf("%.2f\n", metrics.cpu.load_1min);
    } else if (strcmp(metric, "cpu.load.5min") == 0) {
        printf("%.2f\n", metrics.cpu.load_5min);
    } else if (strcmp(metric, "cpu.load.15min") == 0) {
        printf("%.2f\n", metrics.cpu.load_15min);
    } else if (strcmp(metric, "memory.total") == 0) {
        printf("%llu\n", (unsigned long long)metrics.memory.total_bytes);
    } else if (strcmp(metric, "memory.used") == 0) {
        printf("%llu\n", (unsigned long long)metrics.memory.used_bytes);
    } else if (strcmp(metric, "memory.free") == 0) {
        printf("%llu\n", (unsigned long long)metrics.memory.free_bytes);
    } else if (strcmp(metric, "memory.usage") == 0) {
        printf("%.2f\n", metrics.memory.usage_percent);
    } else if (strcmp(metric, "system.hostname") == 0) {
        printf("%s\n", metrics.system.hostname);
    } else if (strcmp(metric, "system.uptime") == 0) {
        printf("%llu\n", (unsigned long long)metrics.system.uptime_seconds);
    } else if (strcmp(metric, "discover") == 0) {
        printf("{\n");
        printf("  \"data\":[\n");
        for (int i = 0; i < metrics.disk_count; i++) {
            if (i > 0) printf(",\n");
            printf("    {\"{#DISK}\":\"%s\", \"{#MOUNT}\":\"%s\"}", 
                   metrics.disks[i].device, metrics.disks[i].mount_point);
        }
        printf("\n  ]\n");
        printf("}\n");
    } else if (strncmp(metric, "disk.total.", 11) == 0) {
        const char* mount = metric + 11;
        for (int i = 0; i < metrics.disk_count; i++) {
            if (strcmp(metrics.disks[i].mount_point, mount) == 0) {
                printf("%llu\n", (unsigned long long)metrics.disks[i].total_bytes);
                goto done;
            }
        }
        printf("ZBX_NOTSUPPORTED\n");
    } else if (strncmp(metric, "disk.used.", 10) == 0) {
        const char* mount = metric + 10;
        for (int i = 0; i < metrics.disk_count; i++) {
            if (strcmp(metrics.disks[i].mount_point, mount) == 0) {
                printf("%llu\n", (unsigned long long)metrics.disks[i].used_bytes);
                goto done;
            }
        }
        printf("ZBX_NOTSUPPORTED\n");
    } else if (strncmp(metric, "disk.free.", 10) == 0) {
        const char* mount = metric + 10;
        for (int i = 0; i < metrics.disk_count; i++) {
            if (strcmp(metrics.disks[i].mount_point, mount) == 0) {
                printf("%llu\n", (unsigned long long)metrics.disks[i].free_bytes);
                goto done;
            }
        }
        printf("ZBX_NOTSUPPORTED\n");
    } else {
        printf("ZBX_NOTSUPPORTED\n");
    }
    
done:
    idcu_server_monitor_destroy(&monitor);
    idcu_log_destroy();
    
    return 0;
}
