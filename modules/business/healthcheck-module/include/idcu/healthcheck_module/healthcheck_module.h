#ifndef IDCU_HEALTHCHECK_MODULE_HEALTHCHECK_MODULE_H
#define IDCU_HEALTHCHECK_MODULE_HEALTHCHECK_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/healthcheck/healthcheck.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_HealthCheck* health_check;
    int initialized;
    void* user_data;
} idcu_HealthCheckModule;

int idcu_healthcheck_module_init(idcu_HealthCheckModule* hcm);
int idcu_healthcheck_module_start(idcu_HealthCheckModule* hcm);
int idcu_healthcheck_module_stop(idcu_HealthCheckModule* hcm);
void idcu_healthcheck_module_destroy(idcu_HealthCheckModule* hcm);

int idcu_healthcheck_module_get_status(idcu_HealthCheckModule* hcm, idcu_HealthCheckResult* result);
idcu_HealthCheck* idcu_healthcheck_module_get_health_check(idcu_HealthCheckModule* hcm);

#ifdef __cplusplus
}
#endif

#endif
