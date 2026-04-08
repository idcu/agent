#include "alert_manager.h"
#include "connection_pool.h"
#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "metrics.h"
#include "module_def.h"
#include <stdio.h>
#include <string.h>

static idcu_ConnectionPool g_http_pool;
static idcu_AlertManager g_alert_mgr;
static idcu_MetricsCollector g_metrics;

static void alert_callback(const idcu_AlertEvent *event, void *user_data) {
    (void)user_data;
    IDCU_LOG_INFO("[ALERT CALLBACK] %s: %s", event->alert_name, event->message);
}

static int advanced_example_init(void) {
    IDCU_LOG_INFO("[advanced_example] initializing...");

    int ret = idcu_connection_pool_init(&g_http_pool, "http_client", IDCU_NET_PROTO_TCP);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("[advanced_example] failed to init connection pool");
        return ret;
    }

    ret = idcu_metrics_init(&g_metrics);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("[advanced_example] failed to init metrics");
        idcu_connection_pool_destroy(&g_http_pool);
        return ret;
    }

    ret = idcu_metrics_register(&g_metrics, "request_count", "Total HTTP requests",
                                IDCU_METRIC_COUNTER);
    idcu_metrics_register(&g_metrics, "response_time_ms", "Response time", IDCU_METRIC_HISTOGRAM);
    idcu_metrics_register(&g_metrics, "active_connections", "Active connections",
                          IDCU_METRIC_GAUGE);

    ret = idcu_alert_manager_init(&g_alert_mgr);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_ERROR("[advanced_example] failed to init alert manager");
        idcu_metrics_destroy(&g_metrics);
        idcu_connection_pool_destroy(&g_http_pool);
        return ret;
    }

    idcu_alert_manager_set_callback(&g_alert_mgr, alert_callback, NULL);

    idcu_AlertRule high_load_rule;
    idcu_alert_rule_init(&high_load_rule, "high_request_rate", IDCU_ALERT_LEVEL_WARNING,
                         "request_count", IDCU_ALERT_COND_GREATER, 1000);
    idcu_alert_rule_set_pending_duration(&high_load_rule, 5000);
    idcu_alert_rule_set_message(&high_load_rule, "High request rate detected!");
    idcu_alert_manager_add_rule(&g_alert_mgr, &high_load_rule);

    idcu_AlertRule slow_response_rule;
    idcu_alert_rule_init(&slow_response_rule, "slow_responses", IDCU_ALERT_LEVEL_ERROR,
                         "response_time_ms", IDCU_ALERT_COND_GREATER, 500);
    idcu_alert_rule_set_message(&slow_response_rule, "Slow responses detected!");
    idcu_alert_manager_add_rule(&g_alert_mgr, &slow_response_rule);

    IDCU_LOG_INFO("[advanced_example] initialized successfully");
    return 0;
}

static int advanced_example_run(void) {
    static uint64_t counter = 0;
    counter++;

    if (counter % 100 == 0) {
        idcu_metrics_inc(&g_metrics, "request_count", 1);
        idcu_metrics_observe(&g_metrics, "response_time_ms", 50 + (counter % 200));

        uint32_t total, in_use, idle;
        idcu_connection_pool_get_stats(&g_http_pool, &total, &in_use, &idle);
        idcu_metrics_set(&g_metrics, "active_connections", in_use);

        idcu_alert_manager_evaluate(&g_alert_mgr, &g_metrics);

        idcu_connection_pool_cleanup_idle(&g_http_pool);
    }

    return 0;
}

static int advanced_example_stop(void) {
    IDCU_LOG_INFO("[advanced_example] stopping...");

    idcu_alert_manager_destroy(&g_alert_mgr);
    idcu_metrics_destroy(&g_metrics);
    idcu_connection_pool_destroy(&g_http_pool);

    IDCU_LOG_INFO("[advanced_example] stopped");
    return 0;
}

IDCU_REGISTER_MODULE(advanced_example, IDCU_MODULE_VERSION(1, 0, 0), advanced_example_init,
                     advanced_example_run, advanced_example_stop);
