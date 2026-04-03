#include "test_framework.h"
#include "metrics.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_metrics_init_destroy(void) {
    MetricsCollector collector;
    int ret = metrics_init(&collector);
    TEST_ASSERT(ret == ERR_OK, "metrics_init should succeed");
    TEST_ASSERT(collector.count == 0, "Metric count should be 0");
    
    metrics_destroy(&collector);
    TEST_PASS();
}

static void test_metrics_register(void) {
    MetricsCollector collector;
    metrics_init(&collector);
    
    int ret = metrics_register(&collector, "test_counter", "A test counter", METRIC_COUNTER);
    TEST_ASSERT(ret == ERR_OK, "Register counter should succeed");
    TEST_ASSERT(collector.count == 1, "Metric count should be 1");
    
    ret = metrics_register(&collector, "test_gauge", "A test gauge", METRIC_GAUGE);
    TEST_ASSERT(ret == ERR_OK, "Register gauge should succeed");
    TEST_ASSERT(collector.count == 2, "Metric count should be 2");
    
    ret = metrics_register(&collector, "test_histogram", "A test histogram", METRIC_HISTOGRAM);
    TEST_ASSERT(ret == ERR_OK, "Register histogram should succeed");
    TEST_ASSERT(collector.count == 3, "Metric count should be 3");
    
    metrics_destroy(&collector);
    TEST_PASS();
}

static void test_metrics_counter(void) {
    MetricsCollector collector;
    metrics_init(&collector);
    
    metrics_register(&collector, "requests", "Total requests", METRIC_COUNTER);
    
    int ret = metrics_inc(&collector, "requests", 1);
    TEST_ASSERT(ret == ERR_OK, "Inc counter should succeed");
    
    uint64_t val = metrics_get(&collector, "requests");
    TEST_ASSERT(val == 1, "Value should be 1");
    
    ret = metrics_inc(&collector, "requests", 5);
    TEST_ASSERT(ret == ERR_OK, "Inc counter by 5 should succeed");
    
    val = metrics_get(&collector, "requests");
    TEST_ASSERT(val == 6, "Value should be 6");
    
    metrics_destroy(&collector);
    TEST_PASS();
}

static void test_metrics_gauge(void) {
    MetricsCollector collector;
    metrics_init(&collector);
    
    metrics_register(&collector, "temperature", "Current temperature", METRIC_GAUGE);
    
    int ret = metrics_set(&collector, "temperature", 25);
    TEST_ASSERT(ret == ERR_OK, "Set gauge should succeed");
    
    uint64_t val = metrics_get(&collector, "temperature");
    TEST_ASSERT(val == 25, "Value should be 25");
    
    ret = metrics_set(&collector, "temperature", 30);
    TEST_ASSERT(ret == ERR_OK, "Set gauge to 30 should succeed");
    
    val = metrics_get(&collector, "temperature");
    TEST_ASSERT(val == 30, "Value should be 30");
    
    metrics_destroy(&collector);
    TEST_PASS();
}

static void test_metrics_histogram(void) {
    MetricsCollector collector;
    metrics_init(&collector);
    
    metrics_register(&collector, "latency", "Request latency", METRIC_HISTOGRAM);
    
    int ret = metrics_observe(&collector, "latency", 100);
    TEST_ASSERT(ret == ERR_OK, "Observe should succeed");
    
    ret = metrics_observe(&collector, "latency", 200);
    TEST_ASSERT(ret == ERR_OK, "Observe 200 should succeed");
    
    ret = metrics_observe(&collector, "latency", 150);
    TEST_ASSERT(ret == ERR_OK, "Observe 150 should succeed");
    
    metrics_destroy(&collector);
    TEST_PASS();
}

static void test_metrics_export(void) {
    MetricsCollector collector;
    metrics_init(&collector);
    
    metrics_register(&collector, "test", "Test metric", METRIC_COUNTER);
    metrics_inc(&collector, "test", 42);
    
    char buffer[512];
    int ret = metrics_export_text(&collector, buffer, sizeof(buffer));
    TEST_ASSERT(ret == ERR_OK, "Export text should succeed");
    TEST_ASSERT(strlen(buffer) > 0, "Exported text should not be empty");
    
    ret = metrics_export_prometheus(&collector, buffer, sizeof(buffer));
    TEST_ASSERT(ret == ERR_OK, "Export prometheus should succeed");
    TEST_ASSERT(strlen(buffer) > 0, "Prometheus export should not be empty");
    
    metrics_destroy(&collector);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Metrics Tests");
    
    test_suite_add_test(&g_suite, "metrics_init_destroy", test_metrics_init_destroy);
    test_suite_add_test(&g_suite, "metrics_register", test_metrics_register);
    test_suite_add_test(&g_suite, "metrics_counter", test_metrics_counter);
    test_suite_add_test(&g_suite, "metrics_gauge", test_metrics_gauge);
    test_suite_add_test(&g_suite, "metrics_histogram", test_metrics_histogram);
    test_suite_add_test(&g_suite, "metrics_export", test_metrics_export);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
