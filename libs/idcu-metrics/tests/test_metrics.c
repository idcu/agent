#include <idcu/testframework/testframework.h>
#include <idcu/metrics/metrics.h>
#include <idcu/metrics/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IDCU_TEST_CASE(metrics, registry_init_destroy) {
    idcu_MetricsRegistry registry;
    int ret = idcu_metrics_registry_init(&registry);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_metrics_registry_destroy(&registry);
}

IDCU_TEST_CASE(metrics, counter) {
    idcu_MetricCounter* counter = idcu_metrics_counter_create("test_counter", "test help", NULL);
    IDCU_TEST_ASSERT(counter != NULL);
    
    idcu_metrics_counter_inc(counter);
    IDCU_TEST_ASSERT_EQUAL(1, idcu_metrics_counter_get(counter));
    
    idcu_metrics_counter_add(counter, 5);
    IDCU_TEST_ASSERT_EQUAL(6, idcu_metrics_counter_get(counter));
    
    idcu_metrics_counter_destroy(counter);
}

IDCU_TEST_CASE(metrics, gauge) {
    idcu_MetricGauge* gauge = idcu_metrics_gauge_create("test_gauge", "test help", NULL);
    IDCU_TEST_ASSERT(gauge != NULL);
    
    idcu_metrics_gauge_set(gauge, 42.5);
    IDCU_TEST_ASSERT_DOUBLE_EQUAL(42.5, idcu_metrics_gauge_get(gauge), 0.001);
    
    idcu_metrics_gauge_inc(gauge);
    IDCU_TEST_ASSERT_DOUBLE_EQUAL(43.5, idcu_metrics_gauge_get(gauge), 0.001);
    
    idcu_metrics_gauge_dec(gauge);
    IDCU_TEST_ASSERT_DOUBLE_EQUAL(42.5, idcu_metrics_gauge_get(gauge), 0.001);
    
    idcu_metrics_gauge_destroy(gauge);
}

IDCU_TEST_CASE(metrics, histogram) {
    double buckets[] = {0.1, 0.5, 1.0, 2.0, 5.0};
    size_t bucket_count = sizeof(buckets) / sizeof(buckets[0]);
    
    idcu_MetricHistogram* histogram = idcu_metrics_histogram_create("test_histogram", "test help", NULL, buckets, bucket_count);
    IDCU_TEST_ASSERT(histogram != NULL);
    
    idcu_metrics_histogram_observe(histogram, 0.3);
    idcu_metrics_histogram_observe(histogram, 1.5);
    
    uint64_t sample_count;
    double sample_sum;
    idcu_metrics_histogram_get_counts(histogram, &sample_count, &sample_sum);
    
    IDCU_TEST_ASSERT_EQUAL(2, sample_count);
    IDCU_TEST_ASSERT_DOUBLE_EQUAL(1.8, sample_sum, 0.001);
    
    idcu_metrics_histogram_destroy(histogram);
}

IDCU_TEST_CASE(metrics, registry_register) {
    idcu_MetricsRegistry registry;
    idcu_metrics_registry_init(&registry);
    
    idcu_MetricCounter* counter = idcu_metrics_counter_create("test_counter", "test help", NULL);
    int ret = idcu_metrics_registry_register_counter(&registry, counter);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_MetricCounter* retrieved = idcu_metrics_registry_get_counter(&registry, "test_counter");
    IDCU_TEST_ASSERT(retrieved == counter);
    
    idcu_metrics_registry_destroy(&registry);
}

IDCU_TEST_CASE(metrics, to_prometheus) {
    idcu_MetricsRegistry registry;
    idcu_metrics_registry_init(&registry);
    
    idcu_MetricCounter* counter = idcu_metrics_counter_create("test_counter", "test help", NULL);
    idcu_metrics_registry_register_counter(&registry, counter);
    idcu_metrics_counter_inc(counter);
    
    char buffer[1024];
    int ret = idcu_metrics_to_prometheus(&registry, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret > 0);
    IDCU_TEST_ASSERT(strstr(buffer, "test_counter") != NULL);
    
    idcu_metrics_registry_destroy(&registry);
}

IDCU_TEST_CASE(metrics, to_json) {
    idcu_MetricsRegistry registry;
    idcu_metrics_registry_init(&registry);
    
    idcu_MetricCounter* counter = idcu_metrics_counter_create("test_counter", "test help", NULL);
    idcu_metrics_registry_register_counter(&registry, counter);
    idcu_metrics_counter_inc(counter);
    
    char buffer[1024];
    int ret = idcu_metrics_to_json(&registry, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret > 0);
    IDCU_TEST_ASSERT(strstr(buffer, "test_counter") != NULL);
    
    idcu_metrics_registry_destroy(&registry);
}

IDCU_TEST_CASE(metrics, labels) {
    idcu_MetricLabels labels;
    int ret = idcu_metrics_labels_init(&labels);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    ret = idcu_metrics_labels_add(&labels, "key1", "value1");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_metrics_labels_clear(&labels);
    
    idcu_MetricCounter* counter = idcu_metrics_counter_create("test_counter", "test help", &labels);
    IDCU_TEST_ASSERT(counter != NULL);
    
    idcu_metrics_counter_destroy(counter);
}

int main(void) {
    return idcu_test_run_all();
}
