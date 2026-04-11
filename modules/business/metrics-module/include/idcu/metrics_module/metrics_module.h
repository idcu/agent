#ifndef IDCU_METRICS_MODULE_METRICS_MODULE_H
#define IDCU_METRICS_MODULE_METRICS_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/metrics/metrics.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    idcu_MetricsRegistry* registry;
    int initialized;
    void* user_data;
} idcu_MetricsModule;

int idcu_metrics_module_init(idcu_MetricsModule* mm);
int idcu_metrics_module_start(idcu_MetricsModule* mm);
int idcu_metrics_module_stop(idcu_MetricsModule* mm);
void idcu_metrics_module_destroy(idcu_MetricsModule* mm);

int idcu_metrics_module_register_counter(idcu_MetricsModule* mm, const char* name, const char* help);
int idcu_metrics_module_register_gauge(idcu_MetricsModule* mm, const char* name, const char* help);
int idcu_metrics_module_inc(idcu_MetricsModule* mm, const char* name, double value);
int idcu_metrics_module_set(idcu_MetricsModule* mm, const char* name, double value);
int idcu_metrics_module_export_prometheus(idcu_MetricsModule* mm, char* buffer, size_t buffer_size);

idcu_MetricsRegistry* idcu_metrics_module_get_registry(idcu_MetricsModule* mm);

#ifdef __cplusplus
}
#endif

#endif
