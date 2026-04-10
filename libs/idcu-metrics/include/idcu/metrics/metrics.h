#ifndef IDCU_METRICS_METRICS_H
#define IDCU_METRICS_METRICS_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/metrics/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_metrics_init(idcu_Metrics_Context** ctx);
void idcu_metrics_destroy(idcu_Metrics_Context* ctx);
int idcu_metrics_is_initialized(idcu_Metrics_Context* ctx);
uint64_t idcu_metrics_get_operation_count(idcu_Metrics_Context* ctx);
uint64_t idcu_metrics_get_error_count(idcu_Metrics_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
