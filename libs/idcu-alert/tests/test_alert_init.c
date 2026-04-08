#include "idcu/alert/alert.h"
#include "idcu/log/log.h"
#include <stdio.h>

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    printf("Testing idcu-alert initialization...\n");

    idcu_AlertManager mgr;
    int ret = idcu_alert_manager_init(&mgr);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("FAILED: Alert manager init failed with code %d\n", ret);
        return 1;
    }

    printf("SUCCESS: Alert manager initialized\n");

    idcu_alert_manager_destroy(&mgr);
    printf("SUCCESS: Alert manager destroyed\n");

    printf("All tests passed!\n");
    return 0;
}
