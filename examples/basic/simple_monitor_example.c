/**
 * @file simple_monitor_example.c
 * @brief 简单的监控服务使用示例
 * 
 * 演示如何使用监控服务的健康检查和指标收集功能
 */

#include "../modules/services/monitor/include/health_check.h"
#include "../modules/services/monitor/include/metrics.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

static volatile int g_running = 1;

void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
    printf("\nShutting down...\n");
}

void health_status_callback(uint32_t module_id, HealthStatus old_status, HealthStatus new_status, void* user_data) {
    (void)user_data;
    printf("[Health] Module %u: %d -> %d\n", module_id, old_status, new_status);
}

int main(void) {
    printf("====================================\n");
    printf("IDCU Monitor Service Example\n");
    printf("====================================\n\n");

    signal(SIGINT, signal_handler);
    
    #ifdef _WIN32
    signal(SIGBREAK, signal_handler);
    #endif

    printf("Step 1: Initializing metrics collector...\n");
    idcu_MetricsCollector metrics;
    int ret = idcu_metrics_init(&metrics);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to initialize metrics collector\n");
        return 1;
    }
    printf("Metrics collector initialized\n\n");

    printf("Step 2: Registering metrics...\n");
    idcu_metrics_register(&metrics, "requests_total", "Total number of requests", IDCU_METRIC_COUNTER);
    idcu_metrics_register(&metrics, "active_connections", "Current active connections", IDCU_METRIC_GAUGE);
    idcu_metrics_register(&metrics, "response_time_ms", "Response time in milliseconds", IDCU_METRIC_HISTOGRAM);
    printf("Metrics registered\n\n");

    printf("Step 3: Initializing health monitor...\n");
    HealthMonitor health_monitor;
    health_monitor_init(&health_monitor);
    health_set_callback(&health_monitor, health_status_callback, NULL);
    health_register_module(&health_monitor, 1);
    health_register_module(&health_monitor, 2);
    printf("Health monitor initialized\n\n");

    printf("Example is running! Press Ctrl+C to stop\n\n");

    uint64_t counter = 0;
    while (g_running) {
        counter++;

        if (counter % 50 == 0) {
            health_update_heartbeat(&health_monitor, 1);
            
            if (counter % 100 == 0) {
                health_update_heartbeat(&health_monitor, 2);
            }
        }

        idcu_metrics_inc(&metrics, "requests_total", 1);
        idcu_metrics_set(&metrics, "active_connections", counter % 10);
        idcu_metrics_observe(&metrics, "response_time_ms", 10 + (counter % 50));

        if (counter % 1000000 == 0) {
            char buffer[2048];
            printf("\n--- Metrics Snapshot ---\n");
            idcu_metrics_export_text(&metrics, buffer, sizeof(buffer));
            printf("%s", buffer);
            printf("------------------------\n\n");
        }

        SLEEP_MS(10);
    }

    printf("\nCleaning up...\n");
    
    health_monitor_destroy(&health_monitor);
    idcu_metrics_destroy(&metrics);
    
    printf("Example completed\n");
    return 0;
}
