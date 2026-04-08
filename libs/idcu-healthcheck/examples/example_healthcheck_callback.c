#include "idcu/healthcheck/healthcheck.h"
#include "idcu/log/log.h"
#include <stdio.h>

void health_status_callback(uint32_t module_id, idcu_HealthStatus old_status,
                            idcu_HealthStatus new_status, void *user_data) {
    printf("[Callback] Module %u status changed: %s -> %s\n", module_id,
           idcu_health_status_to_string(old_status), idcu_health_status_to_string(new_status));
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);

    idcu_health_set_callback(&monitor, health_status_callback, NULL);
    idcu_health_set_thresholds(&monitor, 10000, 2, 5);

    printf("Registering module 201...\n");
    idcu_health_register_module(&monitor, 201);

    printf("Reporting errors...\n");
    for (int i = 0; i < 3; i++) {
        idcu_health_report_error(&monitor, 201);
    }

    printf("Reporting restart...\n");
    idcu_health_report_restart(&monitor, 201);

    idcu_health_monitor_destroy(&monitor);
    return 0;
}
