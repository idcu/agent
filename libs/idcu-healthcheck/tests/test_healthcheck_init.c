#include "idcu/healthcheck/healthcheck.h"
#include "idcu/log/log.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_HealthMonitor monitor;
    int ret = idcu_health_monitor_init(&monitor);
    assert(ret == IDCU_ERR_SUCCESS && "Init should succeed");

    ret = idcu_health_register_module(&monitor, 1);
    assert(ret == IDCU_ERR_SUCCESS && "Register module 1 should succeed");

    ret = idcu_health_update_heartbeat(&monitor, 1);
    assert(ret == IDCU_ERR_SUCCESS && "Update heartbeat should succeed");

    idcu_HealthStatus status = idcu_health_get_status(&monitor, 1);
    assert(status == IDCU_HEALTH_HEALTHY && "Status should be HEALTHY");

    idcu_health_monitor_destroy(&monitor);

    printf("Test passed!\n");
    return 0;
}
