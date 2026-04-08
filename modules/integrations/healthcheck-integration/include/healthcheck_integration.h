#ifndef IDCU_INTEGRATIONS_HEALTHCHECK_INTEGRATION_H
#define IDCU_INTEGRATIONS_HEALTHCHECK_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/healthcheck/healthcheck.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int initialized;
    } idcu_HealthcheckIntegration;

    int  idcu_healthcheck_integration_init(idcu_HealthcheckIntegration* integration);
    void idcu_healthcheck_integration_destroy(idcu_HealthcheckIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_INTEGRATIONS_HEALTHCHECK_INTEGRATION_H
