#ifndef METRICS_H
#define METRICS_H

#include "error_code.h"
#include "lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    METRIC_COUNTER = 0,
    METRIC_GAUGE,
    METRIC_HISTOGRAM
} MetricType;

typedef struct {
    char name[64];
    char desc[128];
    MetricType type;
    uint64_t value;
    uint64_t min;
    uint64_t max;
    uint64_t sum;
    uint64_t count;
} Metric;

#define MAX_METRICS 64

typedef struct {
    Metric metrics[MAX_METRICS];
    uint32_t count;
    Mutex lock;
} MetricsCollector;

int metrics_init(MetricsCollector* collector);
void metrics_destroy(MetricsCollector* collector);
int metrics_register(MetricsCollector* collector, const char* name, const char* desc, MetricType type);
int metrics_inc(MetricsCollector* collector, const char* name, uint64_t value);
int metrics_set(MetricsCollector* collector, const char* name, uint64_t value);
int metrics_observe(MetricsCollector* collector, const char* name, uint64_t value);
uint64_t metrics_get(MetricsCollector* collector, const char* name);
int metrics_export_text(MetricsCollector* collector, char* buffer, size_t buffer_size);
int metrics_export_prometheus(MetricsCollector* collector, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
