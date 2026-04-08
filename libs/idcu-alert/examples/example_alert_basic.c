#include "idcu/alert/alert.h"
#include "idcu/alert/notifier.h"
#include "idcu/log/log.h"
#include "idcu/metrics/metrics.h"
#include <stdio.h>
#include <unistd.h>

static void alert_callback(const idcu_AlertEvent *event, void *user_data) {
    (void)user_data;
    printf("Alert event received: %s [%s]\n", event->alert_name,
           event->state == IDCU_ALERT_STATE_FIRING ? "FIRING" : "RESOLVED");
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    printf("=== idcu-alert Basic Example ===\n\n");

    idcu_MetricsCollector metrics;
    idcu_metrics_init(&metrics);

    idcu_AlertManager mgr;
    idcu_alert_manager_init(&mgr);

    idcu_alert_manager_set_callback(&mgr, alert_callback, NULL);

    idcu_AlertRule rule;
    idcu_alert_rule_init(&rule, "cpu_high", IDCU_ALERT_LEVEL_WARNING, "cpu_usage",
                         IDCU_ALERT_COND_GREATER, 80);
    idcu_alert_rule_set_message(&rule, "CPU usage is high!");
    idcu_alert_rule_set_pending_duration(&rule, 1000);

    idcu_alert_manager_add_rule(&mgr, &rule);

    idcu_Notifier notifier;
    idcu_notifier_init(&notifier);
    idcu_notifier_add_log_channel(&notifier);

    idcu_alert_manager_set_callback(&mgr, idcu_notifier_alert_callback, &notifier);

    printf("Setting cpu_usage to 90...\n");
    idcu_metrics_set(&metrics, "cpu_usage", 90);

    printf("Evaluating rules...\n");
    idcu_alert_manager_evaluate(&mgr, &metrics);

    printf("\nSetting cpu_usage back to 50...\n");
    idcu_metrics_set(&metrics, "cpu_usage", 50);

    printf("Evaluating rules again...\n");
    idcu_alert_manager_evaluate(&mgr, &metrics);

    idcu_notifier_destroy(&notifier);
    idcu_alert_manager_destroy(&mgr);
    idcu_metrics_destroy(&metrics);

    printf("\n=== Example completed ===\n");
    return 0;
}
