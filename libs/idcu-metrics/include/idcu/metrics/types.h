#ifndef IDCU_METRICS_TYPES_H
#define IDCU_METRICS_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/vector.h>
#include <idcu/common/hash_map.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_METRIC_NAME_MAX    64
#define IDCU_METRIC_HELP_MAX    256
#define IDCU_METRIC_LABEL_KEY_MAX  64
#define IDCU_METRIC_LABEL_VALUE_MAX  128
#define IDCU_METRIC_LABELS_MAX  16
#define IDCU_METRIC_BUCKETS_MAX  32

typedef enum
{
    IDCU_METRIC_TYPE_COUNTER = 0,
    IDCU_METRIC_TYPE_GAUGE,
    IDCU_METRIC_TYPE_HISTOGRAM,
    IDCU_METRIC_TYPE_SUMMARY
} idcu_MetricType;

typedef struct
{
    char name[IDCU_METRIC_LABEL_KEY_MAX];
    char value[IDCU_METRIC_LABEL_VALUE_MAX];
} idcu_MetricLabel;

typedef struct
{
    idcu_MetricLabel labels[IDCU_METRIC_LABELS_MAX];
    size_t label_count;
} idcu_MetricLabels;

// Summary metric (quantiles)
typedef struct idcu_MetricSummary
{
    char name[IDCU_METRIC_NAME_MAX];
    char help[IDCU_METRIC_HELP_MAX];
    idcu_MetricLabels labels;
    double quantiles[IDCU_METRIC_BUCKETS_MAX];  // e.g., 0.5, 0.9, 0.99
    size_t quantile_count;
    double* values;                               // For quantile calculation
    size_t value_count;
    size_t value_capacity;
    uint64_t sample_count;
    double sample_sum;
    idcu_Mutex lock;
} idcu_MetricSummary;

typedef struct idcu_MetricCounter
{
    char name[IDCU_METRIC_NAME_MAX];
    char help[IDCU_METRIC_HELP_MAX];
    idcu_MetricLabels labels;
    uint64_t value;
    idcu_Mutex lock;
} idcu_MetricCounter;

typedef struct idcu_MetricGauge
{
    char name[IDCU_METRIC_NAME_MAX];
    char help[IDCU_METRIC_HELP_MAX];
    idcu_MetricLabels labels;
    double value;
    idcu_Mutex lock;
} idcu_MetricGauge;

typedef struct idcu_MetricHistogram
{
    char name[IDCU_METRIC_NAME_MAX];
    char help[IDCU_METRIC_HELP_MAX];
    idcu_MetricLabels labels;
    double buckets[IDCU_METRIC_BUCKETS_MAX];
    size_t bucket_count;
    uint64_t bucket_counts[IDCU_METRIC_BUCKETS_MAX];
    uint64_t sample_count;
    double sample_sum;
    idcu_Mutex lock;
} idcu_MetricHistogram;

typedef struct idcu_Metric
{
    idcu_MetricType type;
    union
    {
        idcu_MetricCounter* counter;
        idcu_MetricGauge* gauge;
        idcu_MetricHistogram* histogram;
    } data;
} idcu_Metric;

typedef struct
{
    idcu_Vector metrics;
    idcu_HashMap metrics_by_name;
    idcu_Mutex lock;
    int initialized;
} idcu_MetricsRegistry;

#ifdef __cplusplus
}
#endif

#endif
