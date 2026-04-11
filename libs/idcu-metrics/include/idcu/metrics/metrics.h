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

int  idcu_metrics_registry_init(idcu_MetricsRegistry* registry);
void idcu_metrics_registry_destroy(idcu_MetricsRegistry* registry);

idcu_MetricCounter* idcu_metrics_counter_create(const char* name, const char* help, const idcu_MetricLabels* labels);
void idcu_metrics_counter_destroy(idcu_MetricCounter* counter);
void idcu_metrics_counter_inc(idcu_MetricCounter* counter);
void idcu_metrics_counter_add(idcu_MetricCounter* counter, uint64_t value);
uint64_t idcu_metrics_counter_get(idcu_MetricCounter* counter);

idcu_MetricGauge* idcu_metrics_gauge_create(const char* name, const char* help, const idcu_MetricLabels* labels);
void idcu_metrics_gauge_destroy(idcu_MetricGauge* gauge);
void idcu_metrics_gauge_set(idcu_MetricGauge* gauge, double value);
void idcu_metrics_gauge_inc(idcu_MetricGauge* gauge);
void idcu_metrics_gauge_dec(idcu_MetricGauge* gauge);
void idcu_metrics_gauge_add(idcu_MetricGauge* gauge, double value);
void idcu_metrics_gauge_sub(idcu_MetricGauge* gauge, double value);
double idcu_metrics_gauge_get(idcu_MetricGauge* gauge);

idcu_MetricHistogram* idcu_metrics_histogram_create(const char* name, const char* help, const idcu_MetricLabels* labels,
                                                      const double* buckets, size_t bucket_count);
void idcu_metrics_histogram_destroy(idcu_MetricHistogram* histogram);
void idcu_metrics_histogram_observe(idcu_MetricHistogram* histogram, double value);
void idcu_metrics_histogram_get_counts(idcu_MetricHistogram* histogram, uint64_t* sample_count, double* sample_sum);

int idcu_metrics_registry_register_counter(idcu_MetricsRegistry* registry, idcu_MetricCounter* counter);
int idcu_metrics_registry_register_gauge(idcu_MetricsRegistry* registry, idcu_MetricGauge* gauge);
int idcu_metrics_registry_register_histogram(idcu_MetricsRegistry* registry, idcu_MetricHistogram* histogram);

int idcu_metrics_registry_unregister(idcu_MetricsRegistry* registry, const char* name);

idcu_MetricCounter* idcu_metrics_registry_get_counter(idcu_MetricsRegistry* registry, const char* name);
idcu_MetricGauge* idcu_metrics_registry_get_gauge(idcu_MetricsRegistry* registry, const char* name);
idcu_MetricHistogram* idcu_metrics_registry_get_histogram(idcu_MetricsRegistry* registry, const char* name);

int idcu_metrics_to_prometheus(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size);
int idcu_metrics_to_json(const idcu_MetricsRegistry* registry, char* buffer, size_t buffer_size);

idcu_MetricSummary* idcu_metrics_summary_create(const char* name, const char* help, const idcu_MetricLabels* labels,
                                                    const double* quantiles, size_t quantile_count);
void idcu_metrics_summary_destroy(idcu_MetricSummary* summary);
void idcu_metrics_summary_observe(idcu_MetricSummary* summary, double value);
void idcu_metrics_summary_get(idcu_MetricSummary* summary, uint64_t* sample_count, double* sample_sum,
                                double* out_quantiles, size_t quantile_count);

int idcu_metrics_registry_register_summary(idcu_MetricsRegistry* registry, idcu_MetricSummary* summary);
idcu_MetricSummary* idcu_metrics_registry_get_summary(idcu_MetricsRegistry* registry, const char* name);

// Persistence
int idcu_metrics_save_to_file(const idcu_MetricsRegistry* registry, const char* filepath);
int idcu_metrics_load_from_file(idcu_MetricsRegistry* registry, const char* filepath);

int idcu_metrics_labels_init(idcu_MetricLabels* labels);
int idcu_metrics_labels_add(idcu_MetricLabels* labels, const char* name, const char* value);
void idcu_metrics_labels_clear(idcu_MetricLabels* labels);

#ifdef __cplusplus
}
#endif

#endif
