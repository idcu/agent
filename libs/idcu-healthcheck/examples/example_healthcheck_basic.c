#include "idcu/healthcheck/healthcheck.h"
#include "idcu/log/log.h"
#include <stdio.h>

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    printf("Registering modules...\n");
    idcu_health_register_module(&monitor, 101);
    idcu_health_register_module(&monitor, 102);
    idcu_health_register_module(&monitor, 103);
    
    printf("Updating heartbeats...\n");
    idcu_health_update_heartbeat(&monitor, 101);
    idcu_health_update_heartbeat(&monitor, 102);
    
    printf("Reporting error for module 103...\n");
    idcu_health_report_error(&monitor, 103);
    
    idcu_HealthSummary summary;
    idcu_health_get_summary(&monitor, &summary);
    
    printf("\nHealth Summary:\n");
    printf("  Total: %u\n", summary.total_count);
    printf("  Healthy: %u\n", summary.healthy_count);
    printf("  Warning: %u\n", summary.warning_count);
    printf("  Critical: %u\n", summary.critical_count);
    printf("  Dead: %u\n", summary.dead_count);
    
    idcu_HealthStatus overall = idcu_health_get_overall_status(&monitor);
    printf("\nOverall Status: %s\n", idcu_health_status_to_string(overall));
    
    idcu_health_monitor_destroy(&monitor);
    return 0;
}
