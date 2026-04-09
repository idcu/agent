#ifndef IDCU_METRICS_METRICS_H
#define IDCU_METRICS_METRICS_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/metrics/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_metrics_init(idcu_Metrics_Context** ctx);
void idcu_metrics_destroy(idcu_Metrics_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
