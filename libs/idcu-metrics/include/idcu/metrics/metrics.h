#ifndef IDCU_METRICS_H
#define IDCU_METRICS_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/string_buf.h"
#include "idcu/common/vector.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_METRIC_COUNTER = 0,
    IDCU_METRIC_GAUGE,
    IDCU_METRIC_HISTOGRAM
} idcu_MetricType;

typedef struct {
    char name[64];
    char desc[128];
    idcu_MetricType type;
    uint64_t value;
    uint64_t min;
    uint64_t max;
    uint64_t sum;
    uint64_t count;
} idcu_Metric;

typedef struct {
    idcu_HashMap metrics_map;
    idcu_Vector metrics_list;
    idcu_Mutex lock;
} idcu_MetricsCollector;

int idcu_metrics_init(idcu_MetricsCollector* collector);
void idcu_metrics_destroy(idcu_MetricsCollector* collector);

int idcu_metrics_register(idcu_MetricsCollector* collector, const char* name, 
                           const char* desc, idcu_MetricType type);
int idcu_metrics_inc(idcu_MetricsCollector* collector, const char* name, uint64_t value);
int idcu_metrics_set(idcu_MetricsCollector* collector, const char* name, uint64_t value);
int idcu_metrics_observe(idcu_MetricsCollector* collector, const char* name, uint64_t value);
uint64_t idcu_metrics_get(idcu_MetricsCollector* collector, const char* name);

int idcu_metrics_export_text(idcu_MetricsCollector* collector, idcu_StringBuf* buf);
int idcu_metrics_export_prometheus(idcu_MetricsCollector* collector, idcu_StringBuf* buf);

int idcu_global_metrics_init(void);
void idcu_global_metrics_destroy(void);
idcu_MetricsCollector* idcu_global_metrics_collector(void);
int idcu_global_metrics_register(const char* name, const char* desc, idcu_MetricType type);
int idcu_global_metrics_inc(const char* name, uint64_t value);
int idcu_global_metrics_set(const char* name, uint64_t value);
int idcu_global_metrics_observe(const char* name, uint64_t value);
uint64_t idcu_global_metrics_get(const char* name);
int idcu_global_metrics_export_text(idcu_StringBuf* buf);
int idcu_global_metrics_export_prometheus(idcu_StringBuf* buf);

#ifdef __cplusplus
}
#endif

#endif
