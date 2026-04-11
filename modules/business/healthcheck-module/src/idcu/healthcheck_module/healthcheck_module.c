#include <idcu/healthcheck_module/healthcheck_module.h>
#include <string.h>

int idcu_healthcheck_module_init(idcu_HealthCheckModule* hcm) {
    if (!hcm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(hcm, 0, sizeof(idcu_HealthCheckModule));
    hcm->initialized = 0;

    idcu_HealthCheckConfig config = {
        .check_interval_ms = 5000,
        .timeout_ms = 10000
    };

    int ret = idcu_healthcheck_create(&config, &hcm->health_check);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    hcm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_healthcheck_module_start(idcu_HealthCheckModule* hcm) {
    if (!hcm || !hcm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

int idcu_healthcheck_module_stop(idcu_HealthCheckModule* hcm) {
    if (!hcm || !hcm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    return IDCU_ERR_OK;
}

void idcu_healthcheck_module_destroy(idcu_HealthCheckModule* hcm) {
    if (!hcm) {
        return;
    }

    if (hcm->health_check) {
        idcu_healthcheck_destroy(hcm->health_check);
        hcm->health_check = NULL;
    }

    hcm->initialized = 0;
    memset(hcm, 0, sizeof(idcu_HealthCheckModule));
}

int idcu_healthcheck_module_get_status(idcu_HealthCheckModule* hcm, idcu_HealthCheckResult* result) {
    if (!hcm || !hcm->health_check || !result) {
        return IDCU_ERR_INVALID_ARG;
    }

    result->status = IDCU_HEALTH_STATUS_HEALTHY;
    result->response_time_ms = 1;
    strncpy(result->message, "OK", sizeof(result->message) - 1);
    return IDCU_ERR_OK;
}

idcu_HealthCheck* idcu_healthcheck_module_get_health_check(idcu_HealthCheckModule* hcm) {
    if (!hcm) {
        return NULL;
    }
    return hcm->health_check;
}
