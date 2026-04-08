#ifndef IDCU_INTEGRATIONS_METRICS_INTEGRATION_H
#define IDCU_INTEGRATIONS_METRICS_INTEGRATION_H

#include "idcu/common/error_code.h"
#include "idcu/metrics/metrics.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int                   initialized;
        idcu_MetricsCollector collector;
    } idcu_MetricsIntegration;

    int  idcu_metrics_integration_init(idcu_MetricsIntegration* integration);
    void idcu_metrics_integration_destroy(idcu_MetricsIntegration* integration);

    idcu_MetricsCollector* idcu_metrics_integration_get_collector(
        idcu_MetricsIntegration* integration);

#ifdef __cplusplus
}
#endif

#endif  // IDCU_INTEGRATIONS_METRICS_INTEGRATION_H
