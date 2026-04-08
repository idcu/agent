#include "idcu/alert/alert.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    printf("Testing alert rule management...\n");

    idcu_AlertManager mgr;
    idcu_alert_manager_init(&mgr);

    idcu_AlertRule rule;
    idcu_alert_rule_init(&rule, "cpu_high", IDCU_ALERT_LEVEL_WARNING, "cpu_usage",
                         IDCU_ALERT_COND_GREATER, 90);

    int ret = idcu_alert_manager_add_rule(&mgr, &rule);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("FAILED: Add rule failed with code %d\n", ret);
        idcu_alert_manager_destroy(&mgr);
        return 1;
    }
    printf("SUCCESS: Rule added\n");

    ret = idcu_alert_manager_add_rule(&mgr, &rule);
    if (ret != IDCU_ERR_ALREADY_EXISTS) {
        printf("FAILED: Duplicate rule should return ALREADY_EXISTS, got %d\n", ret);
        idcu_alert_manager_destroy(&mgr);
        return 1;
    }
    printf("SUCCESS: Duplicate rule check passed\n");

    ret = idcu_alert_manager_remove_rule(&mgr, "cpu_high");
    if (ret != IDCU_ERR_SUCCESS) {
        printf("FAILED: Remove rule failed with code %d\n", ret);
        idcu_alert_manager_destroy(&mgr);
        return 1;
    }
    printf("SUCCESS: Rule removed\n");

    idcu_alert_manager_destroy(&mgr);
    printf("All tests passed!\n");
    return 0;
}
