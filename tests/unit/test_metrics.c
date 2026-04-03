#include "test/test_framework.h"
#include "monitor/metrics.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_metrics_init_destroy(void) {
    idcu_MetricsCollector collector;
    int ret = idcu_metrics_init(&collector);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_metrics_init should succeed");
    
    idcu_metrics_destroy(&collector);
    IDCU_TEST_PASS();
}

static void test_metrics_register(void) {
    idcu_MetricsCollector collector;
    idcu_metrics_init(&collector);
    
    int ret = idcu_metrics_register(&collector, "test_counter", "A test counter", IDCU_METRIC_COUNTER);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Register counter should succeed");
    
    ret = idcu_metrics_register(&collector, "test_gauge", "A test gauge", IDCU_METRIC_GAUGE);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Register gauge should succeed");
    
    ret = idcu_metrics_register(&collector, "test_histogram", "A test histogram", IDCU_METRIC_HISTOGRAM);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Register histogram should succeed");
    
    idcu_metrics_destroy(&collector);
    IDCU_TEST_PASS();
}

static void test_metrics_counter(void) {
    idcu_MetricsCollector collector;
    idcu_metrics_init(&collector);
    
    idcu_metrics_register(&collector, "requests", "Total requests", IDCU_METRIC_COUNTER);
    
    int ret = idcu_metrics_inc(&collector, "requests", 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Inc counter should succeed");
    
    uint64_t val = idcu_metrics_get(&collector, "requests");
    IDCU_TEST_ASSERT(val == 1, "Value should be 1");
    
    ret = idcu_metrics_inc(&collector, "requests", 5);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Inc counter by 5 should succeed");
    
    val = idcu_metrics_get(&collector, "requests");
    IDCU_TEST_ASSERT(val == 6, "Value should be 6");
    
    idcu_metrics_destroy(&collector);
    IDCU_TEST_PASS();
}

static void test_metrics_gauge(void) {
    idcu_MetricsCollector collector;
    idcu_metrics_init(&collector);
    
    idcu_metrics_register(&collector, "temperature", "Current temperature", IDCU_METRIC_GAUGE);
    
    int ret = idcu_metrics_set(&collector, "temperature", 25);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set gauge should succeed");
    
    uint64_t val = idcu_metrics_get(&collector, "temperature");
    IDCU_TEST_ASSERT(val == 25, "Value should be 25");
    
    ret = idcu_metrics_set(&collector, "temperature", 30);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set gauge to 30 should succeed");
    
    val = idcu_metrics_get(&collector, "temperature");
    IDCU_TEST_ASSERT(val == 30, "Value should be 30");
    
    idcu_metrics_destroy(&collector);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Metrics Tests");
    
    idcu_test_suite_add_test(&g_suite, "metrics_init_destroy", test_metrics_init_destroy);
    idcu_test_suite_add_test(&g_suite, "metrics_register", test_metrics_register);
    idcu_test_suite_add_test(&g_suite, "metrics_counter", test_metrics_counter);
    idcu_test_suite_add_test(&g_suite, "metrics_gauge", test_metrics_gauge);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
