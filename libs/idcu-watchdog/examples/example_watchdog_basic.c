#include <idcu/common/error_code.h>
#include <idcu/watchdog/watchdog.h>
#include <stdio.h>
#include <unistd.h>

static void on_timeout_callback(void *user_data) { printf("[WATCHDOG] Timeout detected!\n"); }

static void on_restart_callback(void *user_data) { printf("[WATCHDOG] Restart triggered!\n"); }

int main(void) {
    printf("=== idcu-watchdog Basic Example ===\n\n");

    int ret;
    idcu_Watchdog wd;

    printf("1. Initialize Watchdog\n");
    printf("------------------------\n");
    ret = idcu_watchdog_init(&wd, 3000, 3, 60000);
    if (ret != IDCU_ERR_OK) {
        printf("Watchdog init failed: %s\n", idcu_err_to_str(ret));
        return 1;
    }
    printf("Watchdog initialized:\n");
    printf("  - Timeout: 3000ms\n");
    printf("  - Max restarts: 3\n");
    printf("  - Restart window: 60000ms\n\n");

    printf("2. Set Callbacks\n");
    printf("------------------\n");
    ret = idcu_watchdog_set_on_timeout(&wd, on_timeout_callback, NULL);
    if (ret == IDCU_ERR_OK) {
        printf("Timeout callback set\n");
    }

    ret = idcu_watchdog_set_on_restart(&wd, on_restart_callback, NULL);
    if (ret == IDCU_ERR_OK) {
        printf("Restart callback set\n");
    }
    printf("\n");

    printf("3. Get Initial State\n");
    printf("---------------------\n");
    idcu_WatchdogState state = idcu_watchdog_get_state(&wd);
    printf("Initial state: %s\n", state == IDCU_WATCHDOG_STATE_STOPPED   ? "STOPPED"
                                  : state == IDCU_WATCHDOG_STATE_RUNNING ? "RUNNING"
                                                                         : "ERROR");
    printf("Restart count: %u\n", idcu_watchdog_get_restart_count(&wd));
    printf("\n");

    printf("4. Start Watchdog and Send Heartbeats\n");
    printf("---------------------------------------\n");
    ret = idcu_watchdog_start(&wd);
    if (ret != IDCU_ERR_OK) {
        printf("Watchdog start failed: %s\n", idcu_err_to_str(ret));
        idcu_watchdog_destroy(&wd);
        return 1;
    }
    printf("Watchdog started\n");

    state = idcu_watchdog_get_state(&wd);
    printf("State after start: %s\n", state == IDCU_WATCHDOG_STATE_RUNNING ? "RUNNING" : "OTHER");

    printf("Sending heartbeats...\n");
    for (int i = 0; i < 5; i++) {
        ret = idcu_watchdog_heartbeat(&wd);
        if (ret == IDCU_ERR_OK) {
            printf("  Heartbeat %d sent\n", i + 1);
        }
        sleep(1);
    }
    printf("\n");

    printf("5. Stop Watchdog\n");
    printf("------------------\n");
    ret = idcu_watchdog_stop(&wd);
    if (ret == IDCU_ERR_OK) {
        printf("Watchdog stopped\n");
    }

    state = idcu_watchdog_get_state(&wd);
    printf("State after stop: %s\n", state == IDCU_WATCHDOG_STATE_STOPPED ? "STOPPED" : "OTHER");
    printf("\n");

    printf("6. Destroy Watchdog\n");
    printf("---------------------\n");
    idcu_watchdog_destroy(&wd);
    printf("Watchdog destroyed\n\n");

    printf("=== Example Complete ===\n");
    return 0;
}
