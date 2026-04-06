#ifndef IDCU_WATCHDOG_WATCHDOG_H
#define IDCU_WATCHDOG_WATCHDOG_H

#include "idcu/common/error_code.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_WATCHDOG_HEARTBEAT_INTERVAL_MS 1000
#define IDCU_WATCHDOG_DEFAULT_TIMEOUT_MS 5000
#define IDCU_WATCHDOG_MAX_RESTARTS 5
#define IDCU_WATCHDOG_RESTART_WINDOW_MS 60000

typedef enum {
    IDCU_WATCHDOG_STATE_STOPPED = 0,
    IDCU_WATCHDOG_STATE_RUNNING,
    IDCU_WATCHDOG_STATE_ERROR
} idcu_WatchdogState;

typedef struct {
    uint64_t last_heartbeat;
    uint32_t restart_count;
    uint64_t restart_window_start;
    idcu_WatchdogState state;
    volatile int should_exit;
    void* thread_handle;
    uint32_t timeout_ms;
    uint32_t max_restarts;
    uint32_t restart_window_ms;
} idcu_Watchdog;

typedef void (*idcu_WatchdogCallback)(void* user_data);

int idcu_watchdog_init(idcu_Watchdog* wd, uint32_t timeout_ms, uint32_t max_restarts, uint32_t restart_window_ms);
void idcu_watchdog_destroy(idcu_Watchdog* wd);
int idcu_watchdog_start(idcu_Watchdog* wd);
int idcu_watchdog_stop(idcu_Watchdog* wd);
int idcu_watchdog_heartbeat(idcu_Watchdog* wd);
idcu_WatchdogState idcu_watchdog_get_state(idcu_Watchdog* wd);
uint32_t idcu_watchdog_get_restart_count(idcu_Watchdog* wd);
int idcu_watchdog_set_on_timeout(idcu_Watchdog* wd, idcu_WatchdogCallback callback, void* user_data);
int idcu_watchdog_set_on_restart(idcu_Watchdog* wd, idcu_WatchdogCallback callback, void* user_data);

#endif // IDCU_WATCHDOG_WATCHDOG_H
