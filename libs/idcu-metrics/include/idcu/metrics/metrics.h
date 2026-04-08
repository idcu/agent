#ifndef IDCU_METRICS_H
#define IDCU_METRICS_H

#include "idcu/common/error_code.h"
#include "idcu/common/hash_map.h"
#include "idcu/common/lock.h"
#include "idcu/common/string_buf.h"
#include "idcu/common/vector.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define IDCU_METRIC_LABEL_KEY_MAX   64
#define IDCU_METRIC_LABEL_VALUE_MAX 128
#define IDCU_METRIC_MAX_LABELS      8
#define IDCU_HISTOGRAM_MAX_BUCKETS  16

    typedef enum
    {
        IDCU_METRIC_COUNTER = 0,
        IDCU_METRIC_GAUGE,
        IDCU_METRIC_HISTOGRAM,
        IDCU_METRIC_SUMMARY
    } idcu_MetricType;

    typedef struct
    {
        char key[IDCU_METRIC_LABEL_KEY_MAX];
        char value[IDCU_METRIC_LABEL_VALUE_MAX];
    } idcu_MetricLabel;

    typedef struct
    {
        char             name[64];
        char             desc[128];
        idcu_MetricType  type;
        uint64_t         value;
        uint64_t         min;
        uint64_t         max;
        uint64_t         sum;
        uint64_t         count;
        idcu_MetricLabel labels[IDCU_METRIC_MAX_LABELS];
        int              label_count;
        uint64_t         buckets[IDCU_HISTOGRAM_MAX_BUCKETS];
        uint64_t         bucket_counts[IDCU_HISTOGRAM_MAX_BUCKETS];
        int              bucket_count;
    } idcu_Metric;

    typedef struct
    {
        idcu_HashMap metrics_map;
        idcu_Vector  metrics_list;
        idcu_Mutex   lock;
        char         namespace_prefix[64];
    } idcu_MetricsCollector;

    int  idcu_metrics_init(idcu_MetricsCollector* collector);
    void idcu_metrics_destroy(idcu_MetricsCollector* collector);

    int idcu_metrics_set_namespace(idcu_MetricsCollector* collector, const char* ns);
    int idcu_metrics_register(idcu_MetricsCollector* collector, const char* name, const char* desc,
                              idcu_MetricType type);
    int idcu_metrics_register_with_labels(idcu_MetricsCollector* collector, const char* name,
                                          const char* desc, idcu_MetricType type,
                                          const idcu_MetricLabel* labels, int label_count);
    int idcu_metrics_set_histogram_buckets(idcu_MetricsCollector* collector, const char* name,
                                           const uint64_t* buckets, int bucket_count);
    int idcu_metrics_add_label(idcu_MetricsCollector* collector, const char* name, const char* key,
                               const char* value);
    int idcu_metrics_inc(idcu_MetricsCollector* collector, const char* name, uint64_t value);
    int idcu_metrics_dec(idcu_MetricsCollector* collector, const char* name, uint64_t value);
    int idcu_metrics_set(idcu_MetricsCollector* collector, const char* name, uint64_t value);
    int idcu_metrics_observe(idcu_MetricsCollector* collector, const char* name, uint64_t value);
    uint64_t idcu_metrics_get(idcu_MetricsCollector* collector, const char* name);

    int idcu_metrics_export_text(idcu_MetricsCollector* collector, idcu_StringBuf* buf);
    int idcu_metrics_export_prometheus(idcu_MetricsCollector* collector, idcu_StringBuf* buf);
    int idcu_metrics_export_prometheus_with_timestamp(idcu_MetricsCollector* collector,
                                                      idcu_StringBuf* buf, uint64_t timestamp_ms);
    const char* idcu_metrics_get_grafana_dashboard_json(void);

    int                    idcu_global_metrics_init(void);
    void                   idcu_global_metrics_destroy(void);
    idcu_MetricsCollector* idcu_global_metrics_collector(void);
    int                    idcu_global_metrics_set_namespace(const char* ns);
    int idcu_global_metrics_register(const char* name, const char* desc, idcu_MetricType type);
    int idcu_global_metrics_register_with_labels(const char* name, const char* desc,
                                                 idcu_MetricType         type,
                                                 const idcu_MetricLabel* labels, int label_count);
    int idcu_global_metrics_set_histogram_buckets(const char* name, const uint64_t* buckets,
                                                  int bucket_count);
    int idcu_global_metrics_add_label(const char* name, const char* key, const char* value);
    int idcu_global_metrics_inc(const char* name, uint64_t value);
    int idcu_global_metrics_dec(const char* name, uint64_t value);
    int idcu_global_metrics_set(const char* name, uint64_t value);
    int idcu_global_metrics_observe(const char* name, uint64_t value);
    uint64_t idcu_global_metrics_get(const char* name);
    int      idcu_global_metrics_export_text(idcu_StringBuf* buf);
    int      idcu_global_metrics_export_prometheus(idcu_StringBuf* buf);
    int      idcu_global_metrics_export_prometheus_with_timestamp(idcu_StringBuf* buf,
                                                                  uint64_t        timestamp_ms);

#ifdef __cplusplus
}
#endif

#endif
