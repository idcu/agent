#ifndef IDCU_WATCHDOG_WATCHDOG_H
#define IDCU_WATCHDOG_WATCHDOG_H

#include <idcu/common/error_code.h>
#include <idcu/common/vector.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IDCU_WATCHDOG_STATUS_OK = 0,
    IDCU_WATCHDOG_STATUS_WARNING,
    IDCU_WATCHDOG_STATUS_CRITICAL,
    IDCU_WATCHDOG_STATUS_TIMEOUT
} idcu_WatchdogStatus;

typedef void (*idcu_WatchdogTimeoutFunc)(void* user_data, const char* name);

typedef struct idcu_WatchdogTimer
{
    char name[128];
    uint64_t timeout_ms;
    uint64_t last_ping_ms;
    idcu_WatchdogStatus status;
    idcu_WatchdogTimeoutFunc timeout_func;
    void* user_data;
    int enabled;
} idcu_WatchdogTimer;

typedef struct
{
    idcu_Vector timers;
    idcu_Mutex lock;
    int initialized;
    int running;
    uint64_t check_interval_ms;
} idcu_Watchdog;

int  idcu_watchdog_init(idcu_Watchdog* watchdog, uint64_t check_interval_ms);
void idcu_watchdog_destroy(idcu_Watchdog* watchdog);

int  idcu_watchdog_add_timer(idcu_Watchdog* watchdog, const char* name, uint64_t timeout_ms, idcu_WatchdogTimeoutFunc timeout_func, void* user_data);
int  idcu_watchdog_remove_timer(idcu_Watchdog* watchdog, const char* name);
int  idcu_watchdog_ping(idcu_Watchdog* watchdog, const char* name);
int  idcu_watchdog_enable_timer(idcu_Watchdog* watchdog, const char* name);
int  idcu_watchdog_disable_timer(idcu_Watchdog* watchdog, const char* name);

int  idcu_watchdog_check(idcu_Watchdog* watchdog);
idcu_WatchdogStatus idcu_watchdog_get_timer_status(idcu_Watchdog* watchdog, const char* name);
idcu_WatchdogStatus idcu_watchdog_get_overall_status(idcu_Watchdog* watchdog);

const char* idcu_watchdog_status_to_string(idcu_WatchdogStatus status);

#ifdef __cplusplus
}
#endif

#endif
