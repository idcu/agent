#ifndef IDCU_MONITOR_METRICS_H
#define IDCU_MONITOR_METRICS_H

#include "common/error_code.h"
#include "common/lock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_MONITOR_METRICS_H

#define IDCU_METRIC_CORO_TOTAL "coro_total"
#define IDCU_METRIC_CORO_RUNNING "coro_running"
#define IDCU_METRIC_CORO_READY "coro_ready"
#define IDCU_METRIC_CORO_SUSPENDED "coro_suspended"
#define IDCU_METRIC_CORO_SWITCHES "coro_switches_total"
#define IDCU_METRIC_CORO_RUNTIME_US "coro_runtime_us_total"

#define IDCU_METRIC_MSG_SENT "msg_sent_total"
#define IDCU_METRIC_MSG_RECEIVED "msg_received_total"
#define IDCU_METRIC_MSG_BROADCAST "msg_broadcast_total"
#define IDCU_METRIC_MSG_QUEUE_SIZE "msg_queue_size"
#define IDCU_METRIC_MSG_ZEROCOPY_SENT "msg_zerocopy_sent_total"
#define IDCU_METRIC_MSG_BATCH_SENT "msg_batch_sent_total"
#define IDCU_METRIC_MSG_BATCH_RECEIVED "msg_batch_received_total"

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

#define IDCU_MAX_METRICS 64

typedef struct {
    idcu_Metric metrics[IDCU_MAX_METRICS];
    uint32_t count;
    idcu_Mutex lock;
} idcu_MetricsCollector;

int idcu_metrics_init(idcu_MetricsCollector* collector);
void idcu_metrics_destroy(idcu_MetricsCollector* collector);
int idcu_metrics_register(idcu_MetricsCollector* collector, const char* name, const char* desc, idcu_MetricType type);
int idcu_metrics_inc(idcu_MetricsCollector* collector, const char* name, uint64_t value);
int idcu_metrics_set(idcu_MetricsCollector* collector, const char* name, uint64_t value);
int idcu_metrics_observe(idcu_MetricsCollector* collector, const char* name, uint64_t value);
uint64_t idcu_metrics_get(idcu_MetricsCollector* collector, const char* name);
int idcu_metrics_export_text(idcu_MetricsCollector* collector, char* buffer, size_t buffer_size);
int idcu_metrics_export_prometheus(idcu_MetricsCollector* collector, char* buffer, size_t buffer_size);

int idcu_global_metrics_init(void);
void idcu_global_metrics_destroy(void);
idcu_MetricsCollector* idcu_global_metrics_collector(void);
int idcu_global_metrics_register(const char* name, const char* desc, idcu_MetricType type);
int idcu_global_metrics_inc(const char* name, uint64_t value);
int idcu_global_metrics_set(const char* name, uint64_t value);
int idcu_global_metrics_observe(const char* name, uint64_t value);
uint64_t idcu_global_metrics_get(const char* name);
int idcu_global_metrics_export_text(char* buffer, size_t buffer_size);
int idcu_global_metrics_export_prometheus(char* buffer, size_t buffer_size);
int idcu_global_metrics_register_default(void);

#ifdef __cplusplus
}
#endif // IDCU_MONITOR_METRICS_H

#endif // IDCU_MONITOR_METRICS_H
