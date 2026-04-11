#include <idcu/watchdog/watchdog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

static uint64_t get_time_ms(void) {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    return (li.QuadPart - 116444736000000000ULL) / 10000;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

const char* idcu_watchdog_status_to_string(idcu_WatchdogStatus status) {
    switch (status) {
        case IDCU_WATCHDOG_STATUS_OK: return "ok";
        case IDCU_WATCHDOG_STATUS_WARNING: return "warning";
        case IDCU_WATCHDOG_STATUS_CRITICAL: return "critical";
        case IDCU_WATCHDOG_STATUS_TIMEOUT: return "timeout";
        default: return "unknown";
    }
}

static void timer_dtor(void* element) {
    (void)element;
}

int idcu_watchdog_init(idcu_Watchdog* watchdog, uint64_t check_interval_ms) {
    if (!watchdog) {
        return IDCU_ERR_INVALID_ARG;
    }
    memset(watchdog, 0, sizeof(idcu_Watchdog));
    int ret = idcu_vector_init_with_dtor(&watchdog->timers, sizeof(idcu_WatchdogTimer), 8, timer_dtor);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }
    ret = idcu_mutex_init(&watchdog->lock);
    if (ret != IDCU_ERR_OK) {
        idcu_vector_destroy(&watchdog->timers);
        return ret;
    }
    watchdog->check_interval_ms = check_interval_ms;
    watchdog->initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_watchdog_destroy(idcu_Watchdog* watchdog) {
    if (!watchdog) {
        return;
    }
    idcu_mutex_lock(&watchdog->lock);
    idcu_vector_destroy(&watchdog->timers);
    idcu_Mutex lock_copy = watchdog->lock;
    watchdog->initialized = 0;
    watchdog->running = 0;
    idcu_mutex_unlock(&lock_copy);
    idcu_mutex_destroy(&lock_copy);
}

int idcu_watchdog_add_timer(idcu_Watchdog* watchdog, const char* name, uint64_t timeout_ms, idcu_WatchdogTimeoutFunc timeout_func, void* user_data) {
    if (!watchdog || !watchdog->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&watchdog->lock);
    idcu_WatchdogTimer timer;
    memset(&timer, 0, sizeof(timer));
    strncpy(timer.name, name, sizeof(timer.name) - 1);
    timer.timeout_ms = timeout_ms;
    timer.last_ping_ms = get_time_ms();
    timer.status = IDCU_WATCHDOG_STATUS_OK;
    timer.timeout_func = timeout_func;
    timer.user_data = user_data;
    timer.enabled = 1;
    int ret = idcu_vector_push_back(&watchdog->timers, &timer);
    idcu_mutex_unlock(&watchdog->lock);
    return ret;
}

int idcu_watchdog_remove_timer(idcu_Watchdog* watchdog, const char* name) {
    if (!watchdog || !watchdog->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&watchdog->lock);
    size_t count = idcu_vector_size(&watchdog->timers);
    for (size_t i = 0; i < count; i++) {
        idcu_WatchdogTimer* timer = (idcu_WatchdogTimer*)idcu_vector_at(&watchdog->timers, i);
        if (strcmp(timer->name, name) == 0) {
            idcu_vector_erase(&watchdog->timers, i);
            idcu_mutex_unlock(&watchdog->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&watchdog->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_watchdog_ping(idcu_Watchdog* watchdog, const char* name) {
    if (!watchdog || !watchdog->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&watchdog->lock);
    size_t count = idcu_vector_size(&watchdog->timers);
    for (size_t i = 0; i < count; i++) {
        idcu_WatchdogTimer* timer = (idcu_WatchdogTimer*)idcu_vector_at(&watchdog->timers, i);
        if (strcmp(timer->name, name) == 0) {
            timer->last_ping_ms = get_time_ms();
            timer->status = IDCU_WATCHDOG_STATUS_OK;
            idcu_mutex_unlock(&watchdog->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&watchdog->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_watchdog_enable_timer(idcu_Watchdog* watchdog, const char* name) {
    if (!watchdog || !watchdog->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&watchdog->lock);
    size_t count = idcu_vector_size(&watchdog->timers);
    for (size_t i = 0; i < count; i++) {
        idcu_WatchdogTimer* timer = (idcu_WatchdogTimer*)idcu_vector_at(&watchdog->timers, i);
        if (strcmp(timer->name, name) == 0) {
            timer->enabled = 1;
            timer->last_ping_ms = get_time_ms();
            timer->status = IDCU_WATCHDOG_STATUS_OK;
            idcu_mutex_unlock(&watchdog->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&watchdog->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_watchdog_disable_timer(idcu_Watchdog* watchdog, const char* name) {
    if (!watchdog || !watchdog->initialized || !name) {
        return IDCU_ERR_INVALID_ARG;
    }
    idcu_mutex_lock(&watchdog->lock);
    size_t count = idcu_vector_size(&watchdog->timers);
    for (size_t i = 0; i < count; i++) {
        idcu_WatchdogTimer* timer = (idcu_WatchdogTimer*)idcu_vector_at(&watchdog->timers, i);
        if (strcmp(timer->name, name) == 0) {
            timer->enabled = 0;
            idcu_mutex_unlock(&watchdog->lock);
            return IDCU_ERR_OK;
        }
    }
    idcu_mutex_unlock(&watchdog->lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_watchdog_check(idcu_Watchdog* watchdog) {
    if (!watchdog || !watchdog->initialized) {
        return IDCU_ERR_INVALID_ARG;
    }
    uint64_t now = get_time_ms();
    idcu_mutex_lock(&watchdog->lock);
    size_t count = idcu_vector_size(&watchdog->timers);
    for (size_t i = 0; i < count; i++) {
        idcu_WatchdogTimer* timer = (idcu_WatchdogTimer*)idcu_vector_at(&watchdog->timers, i);
        if (!timer->enabled) {
            continue;
        }
        uint64_t elapsed = now - timer->last_ping_ms;
        if (elapsed > timer->timeout_ms) {
            timer->status = IDCU_WATCHDOG_STATUS_TIMEOUT;
            if (timer->timeout_func) {
                timer->timeout_func(timer->user_data, timer->name);
            }
        } else if (elapsed > timer->timeout_ms * 0.8) {
            timer->status = IDCU_WATCHDOG_STATUS_CRITICAL;
        } else if (elapsed > timer->timeout_ms * 0.5) {
            timer->status = IDCU_WATCHDOG_STATUS_WARNING;
        } else {
            timer->status = IDCU_WATCHDOG_STATUS_OK;
        }
    }
    idcu_mutex_unlock(&watchdog->lock);
    return IDCU_ERR_OK;
}

idcu_WatchdogStatus idcu_watchdog_get_timer_status(idcu_Watchdog* watchdog, const char* name) {
    if (!watchdog || !watchdog->initialized || !name) {
        return IDCU_WATCHDOG_STATUS_TIMEOUT;
    }
    idcu_mutex_lock(&watchdog->lock);
    size_t count = idcu_vector_size(&watchdog->timers);
    for (size_t i = 0; i < count; i++) {
        idcu_WatchdogTimer* timer = (idcu_WatchdogTimer*)idcu_vector_at(&watchdog->timers, i);
        if (strcmp(timer->name, name) == 0) {
            idcu_WatchdogStatus status = timer->status;
            idcu_mutex_unlock(&watchdog->lock);
            return status;
        }
    }
    idcu_mutex_unlock(&watchdog->lock);
    return IDCU_WATCHDOG_STATUS_TIMEOUT;
}

idcu_WatchdogStatus idcu_watchdog_get_overall_status(idcu_Watchdog* watchdog) {
    if (!watchdog || !watchdog->initialized) {
        return IDCU_WATCHDOG_STATUS_TIMEOUT;
    }
    idcu_mutex_lock(&watchdog->lock);
    idcu_WatchdogStatus overall = IDCU_WATCHDOG_STATUS_OK;
    size_t count = idcu_vector_size(&watchdog->timers);
    for (size_t i = 0; i < count; i++) {
        idcu_WatchdogTimer* timer = (idcu_WatchdogTimer*)idcu_vector_at(&watchdog->timers, i);
        if (timer->enabled && timer->status > overall) {
            overall = timer->status;
        }
    }
    idcu_mutex_unlock(&watchdog->lock);
    return overall;
}
