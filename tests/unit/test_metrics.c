#include "test/test_framework.h"
#include "metrics.h"
#include "prometheus_exporter.h"
#include "coroutine.h"
#include "msg_bus.h"
#include "network_layer.h"
#include "log.h"
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

static void test_metrics_export_prometheus(void) {
    idcu_MetricsCollector collector;
    idcu_metrics_init(&collector);
    
    idcu_metrics_register(&collector, "http_requests_total", "Total HTTP requests", IDCU_METRIC_COUNTER);
    idcu_metrics_register(&collector, "memory_usage_bytes", "Current memory usage", IDCU_METRIC_GAUGE);
    idcu_metrics_register(&collector, "request_duration_ms", "Request duration", IDCU_METRIC_HISTOGRAM);
    
    idcu_metrics_inc(&collector, "http_requests_total", 100);
    idcu_metrics_set(&collector, "memory_usage_bytes", 1024000);
    idcu_metrics_observe(&collector, "request_duration_ms", 50);
    idcu_metrics_observe(&collector, "request_duration_ms", 100);
    idcu_metrics_observe(&collector, "request_duration_ms", 75);
    
    char buffer[4096];
    int ret = idcu_metrics_export_prometheus(&collector, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Export Prometheus should succeed");
    
    IDCU_TEST_ASSERT(strstr(buffer, "# HELP http_requests_total") != NULL, "Should contain http_requests_total HELP");
    IDCU_TEST_ASSERT(strstr(buffer, "# TYPE http_requests_total counter") != NULL, "Should contain http_requests_total TYPE");
    IDCU_TEST_ASSERT(strstr(buffer, "http_requests_total 100") != NULL, "Should contain http_requests_total value 100");
    
    IDCU_TEST_ASSERT(strstr(buffer, "# HELP memory_usage_bytes") != NULL, "Should contain memory_usage_bytes HELP");
    IDCU_TEST_ASSERT(strstr(buffer, "# TYPE memory_usage_bytes gauge") != NULL, "Should contain memory_usage_bytes TYPE");
    IDCU_TEST_ASSERT(strstr(buffer, "memory_usage_bytes 1024000") != NULL, "Should contain memory_usage_bytes value 1024000");
    
    IDCU_TEST_ASSERT(strstr(buffer, "# HELP request_duration_ms") != NULL, "Should contain request_duration_ms HELP");
    IDCU_TEST_ASSERT(strstr(buffer, "# TYPE request_duration_ms summary") != NULL, "Should contain request_duration_ms TYPE");
    IDCU_TEST_ASSERT(strstr(buffer, "request_duration_ms_sum 225") != NULL, "Should contain request_duration_ms_sum 225");
    IDCU_TEST_ASSERT(strstr(buffer, "request_duration_ms_count 3") != NULL, "Should contain request_duration_ms_count 3");
    
    idcu_metrics_destroy(&collector);
    IDCU_TEST_PASS();
}

static void test_prometheus_exporter_init_destroy(void) {
    idcu_MetricsCollector metrics;
    idcu_metrics_init(&metrics);
    
    idcu_PrometheusExporter exporter;
    int ret = idcu_prometheus_exporter_init(&exporter, &metrics, "127.0.0.1", 9091);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Prometheus exporter init should succeed");
    
    idcu_prometheus_exporter_destroy(&exporter);
    idcu_metrics_destroy(&metrics);
    IDCU_TEST_PASS();
}

static void test_global_metrics_init_destroy(void) {
    int ret = idcu_global_metrics_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Global metrics init should succeed");
    
    ret = idcu_global_metrics_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Re-init should succeed (idempotent)");
    
    idcu_global_metrics_destroy();
    idcu_global_metrics_destroy();
    IDCU_TEST_PASS();
}

static void test_global_metrics_register_default(void) {
    idcu_global_metrics_init();
    
    int ret = idcu_global_metrics_register_default();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Register default metrics should succeed");
    
    uint64_t val = idcu_global_metrics_get(IDCU_METRIC_CORO_TOTAL);
    IDCU_TEST_ASSERT(val == 0, "coro_total should be 0 initially");
    
    val = idcu_global_metrics_get(IDCU_METRIC_MSG_SENT);
    IDCU_TEST_ASSERT(val == 0, "msg_sent should be 0 initially");
    
    idcu_global_metrics_destroy();
    IDCU_TEST_PASS();
}

static idcu_CoroState test_coro_func(idcu_Coroutine* coro) {
    (void)coro;
    return IDCU_CORO_FINISHED;
}

static void test_coro_metrics_integration(void) {
    idcu_global_metrics_init();
    idcu_global_metrics_register_default();
    
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    uint64_t total_before = idcu_global_metrics_get(IDCU_METRIC_CORO_TOTAL);
    IDCU_TEST_ASSERT(total_before == 0, "coro_total should be 0");
    
    int coro_id = idcu_coro_create(&sched, test_coro_func, 10, 10, NULL);
    IDCU_TEST_ASSERT(coro_id > 0, "Create coroutine should succeed");
    
    uint64_t total_after = idcu_global_metrics_get(IDCU_METRIC_CORO_TOTAL);
    IDCU_TEST_ASSERT(total_after == 1, "coro_total should be 1");
    
    uint64_t ready_before = idcu_global_metrics_get(IDCU_METRIC_CORO_READY);
    IDCU_TEST_ASSERT(ready_before == 1, "coro_ready should be 1");
    
    idcu_coro_sched_run(&sched);
    
    uint64_t switches = idcu_global_metrics_get(IDCU_METRIC_CORO_SWITCHES);
    IDCU_TEST_ASSERT(switches == 1, "coro_switches should be 1");
    
    uint64_t runtime = idcu_global_metrics_get(IDCU_METRIC_CORO_RUNTIME_US);
    IDCU_TEST_ASSERT(runtime > 0, "coro_runtime should be > 0");
    
    idcu_coro_destroy(&sched, coro_id);
    
    uint64_t total_final = idcu_global_metrics_get(IDCU_METRIC_CORO_TOTAL);
    IDCU_TEST_ASSERT(total_final == 0, "coro_total should be 0 after destroy");
    
    idcu_coro_sched_destroy(&sched);
    idcu_global_metrics_destroy();
    IDCU_TEST_PASS();
}

static void test_msg_bus_metrics_integration(void) {
    idcu_global_metrics_init();
    idcu_global_metrics_register_default();
    
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    uint64_t sent_before = idcu_global_metrics_get(IDCU_METRIC_MSG_SENT);
    IDCU_TEST_ASSERT(sent_before == 0, "msg_sent should be 0");
    
    idcu_StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    int ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Send message should succeed");
    
    uint64_t sent_after = idcu_global_metrics_get(IDCU_METRIC_MSG_SENT);
    IDCU_TEST_ASSERT(sent_after == 1, "msg_sent should be 1");
    
    uint64_t queue_size = idcu_global_metrics_get(IDCU_METRIC_MSG_QUEUE_SIZE);
    IDCU_TEST_ASSERT(queue_size == 1, "msg_queue_size should be 1");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Receive message should succeed");
    
    uint64_t received = idcu_global_metrics_get(IDCU_METRIC_MSG_RECEIVED);
    IDCU_TEST_ASSERT(received == 1, "msg_received should be 1");
    
    queue_size = idcu_global_metrics_get(IDCU_METRIC_MSG_QUEUE_SIZE);
    IDCU_TEST_ASSERT(queue_size == 0, "msg_queue_size should be 0");
    
    ret = idcu_msg_broadcast(&bus, 1, IDCU_MSG_PRIO_NORMAL, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Broadcast message should succeed");
    
    uint64_t broadcast = idcu_global_metrics_get(IDCU_METRIC_MSG_BROADCAST);
    IDCU_TEST_ASSERT(broadcast == 1, "msg_broadcast should be 1");
    
    idcu_msg_bus_destroy(&bus);
    idcu_global_metrics_destroy();
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    idcu_network_init();
    
    idcu_test_suite_init(&g_suite, "Metrics Tests");
    
    idcu_test_suite_add_test(&g_suite, "metrics_init_destroy", test_metrics_init_destroy);
    idcu_test_suite_add_test(&g_suite, "metrics_register", test_metrics_register);
    idcu_test_suite_add_test(&g_suite, "metrics_counter", test_metrics_counter);
    idcu_test_suite_add_test(&g_suite, "metrics_gauge", test_metrics_gauge);
    idcu_test_suite_add_test(&g_suite, "metrics_export_prometheus", test_metrics_export_prometheus);
    idcu_test_suite_add_test(&g_suite, "prometheus_exporter_init_destroy", test_prometheus_exporter_init_destroy);
    idcu_test_suite_add_test(&g_suite, "global_metrics_init_destroy", test_global_metrics_init_destroy);
    idcu_test_suite_add_test(&g_suite, "global_metrics_register_default", test_global_metrics_register_default);
    idcu_test_suite_add_test(&g_suite, "coro_metrics_integration", test_coro_metrics_integration);
    idcu_test_suite_add_test(&g_suite, "msg_bus_metrics_integration", test_msg_bus_metrics_integration);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    idcu_network_cleanup();
    return failures > 0 ? 1 : 0;
}
