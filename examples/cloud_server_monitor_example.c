#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "idcu/server_monitor/server_monitor.h"
#include "idcu/scheduler/scheduler.h"
#include "idcu/log/log.h"

static idcu_ServerMonitor g_monitor;
static idcu_Scheduler g_scheduler;
static int g_running = 1;

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static void collect_and_print_metrics(void* user_data) {
    idcu_ServerMetrics metrics;
    if (idcu_server_monitor_collect(&g_monitor, &metrics) != IDCU_ERR_OK) {
        printf("Failed to collect metrics\n");
        return;
    }
    
    printf("\n=== Server Metrics ===\n");
    printf("Hostname: %s\n", metrics.system.hostname);
    printf("OS: %s %s\n", metrics.system.os_name, metrics.system.os_version);
    printf("Uptime: %s\n", metrics.system.uptime);
    printf("\n");
    
    printf("CPU Load:\n");
    printf("  1min:  %.2f\n", metrics.cpu.load_1min);
    printf("  5min:  %.2f\n", metrics.cpu.load_5min);
    printf("  15min: %.2f\n", metrics.cpu.load_15min);
    printf("\n");
    
    printf("Memory:\n");
    printf("  Total: %.2f GB\n", (double)metrics.memory.total_bytes / (1024 * 1024 * 1024));
    printf("  Used:  %.2f GB (%.1f%%)\n", 
           (double)metrics.memory.used_bytes / (1024 * 1024 * 1024),
           metrics.memory.usage_percent);
    printf("  Free:  %.2f GB\n", (double)metrics.memory.free_bytes / (1024 * 1024 * 1024));
    printf("\n");
    
    printf("Disks:\n");
    for (int i = 0; i < metrics.disk_count; i++) {
        printf("  %s (%s):\n", metrics.disks[i].device, metrics.disks[i].mount_point);
        printf("    Total: %.2f GB\n", (double)metrics.disks[i].total_bytes / (1024 * 1024 * 1024));
        printf("    Used:  %.2f GB\n", (double)metrics.disks[i].used_bytes / (1024 * 1024 * 1024));
        printf("    Free:  %.2f GB\n", (double)metrics.disks[i].free_bytes / (1024 * 1024 * 1024));
    }
    printf("=====================\n\n");
}

static void export_json(void* user_data) {
    char buffer[8192];
    size_t size;
    
    if (idcu_server_monitor_export_json(&g_monitor, buffer, sizeof(buffer), &size) == IDCU_ERR_OK) {
        printf("\n=== JSON Export ===\n");
        printf("%s\n", buffer);
        printf("===================\n\n");
    }
}

static void export_prometheus(void* user_data) {
    char buffer[8192];
    size_t size;
    
    if (idcu_server_monitor_export_prometheus(&g_monitor, buffer, sizeof(buffer), &size) == IDCU_ERR_OK) {
        printf("\n=== Prometheus Export ===\n");
        printf("%s", buffer);
        printf("========================\n\n");
    }
}

int main(int argc, char* argv[]) {
    printf("=== Cloud Server Monitor Example ===\n\n");
    
    idcu_log_init();
    idcu_server_monitor_init(&g_monitor);
    idcu_scheduler_init(&g_scheduler);
    
    idcu_scheduler_start(&g_scheduler);
    
    idcu_scheduler_add_interval_task(&g_scheduler, "collect-metrics", 5000, 1, 
                                      collect_and_print_metrics, NULL);
    idcu_scheduler_add_interval_task(&g_scheduler, "export-json", 15000, 1, 
                                      export_json, NULL);
    idcu_scheduler_add_interval_task(&g_scheduler, "export-prometheus", 20000, 1, 
                                      export_prometheus, NULL);
    
    printf("Monitoring started. Collecting metrics every 5 seconds.\n");
    printf("Press Ctrl+C to stop.\n\n");
    
    while (g_running) {
        idcu_scheduler_process(&g_scheduler);
        sleep_ms(100);
    }
    
    idcu_scheduler_stop(&g_scheduler);
    idcu_scheduler_destroy(&g_scheduler);
    idcu_server_monitor_destroy(&g_monitor);
    idcu_log_destroy();
    
    printf("\nExample completed.\n");
    return 0;
}
