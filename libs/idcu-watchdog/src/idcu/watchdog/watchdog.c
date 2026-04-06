#include "idcu/watchdog/watchdog.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#endif

static idcu_WatchdogCallback g_on_timeout = NULL;
static void* g_on_timeout_data = NULL;
static idcu_WatchdogCallback g_on_restart = NULL;
static void* g_on_restart_data = NULL;

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

static void sleep_ms(uint32_t ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

#ifdef _WIN32
static DWORD WINAPI watchdog_thread(LPVOID param) {
#else
static void* watchdog_thread(void* param) {
#endif
    idcu_Watchdog* wd = (idcu_Watchdog*)param;
    uint64_t current_time;
    
    idcu_log_info("Watchdog thread started");
    
    while (!wd->should_exit) {
        current_time = get_current_time_ms();
        
        if (current_time - wd->last_heartbeat > wd->timeout_ms) {
            idcu_log_error("Watchdog timeout detected! Last heartbeat: %llu ms ago", 
                          (unsigned long long)(current_time - wd->last_heartbeat));
            
            if (g_on_timeout) {
                g_on_timeout(g_on_timeout_data);
            }
            
            uint64_t window_elapsed = current_time - wd->restart_window_start;
            if (window_elapsed > wd->restart_window_ms) {
                wd->restart_count = 0;
                wd->restart_window_start = current_time;
            }
            
            if (wd->restart_count < wd->max_restarts) {
                wd->restart_count++;
                idcu_log_warning("Attempting restart %u/%u", wd->restart_count, wd->max_restarts);
                
                if (g_on_restart) {
                    g_on_restart(g_on_restart_data);
                }
                
                wd->last_heartbeat = get_current_time_ms();
            } else {
                idcu_log_error("Max restart attempts (%u) reached in %u ms window. Giving up.", 
                              wd->max_restarts, wd->restart_window_ms);
                wd->state = IDCU_WATCHDOG_STATE_ERROR;
                break;
            }
        }
        
        sleep_ms(IDCU_WATCHDOG_HEARTBEAT_INTERVAL_MS);
    }
    
    idcu_log_info("Watchdog thread stopped");
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int idcu_watchdog_init(idcu_Watchdog* wd, uint32_t timeout_ms, uint32_t max_restarts, uint32_t restart_window_ms) {
    if (!wd) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(wd, 0, sizeof(idcu_Watchdog));
    wd->state = IDCU_WATCHDOG_STATE_STOPPED;
    wd->timeout_ms = timeout_ms > 0 ? timeout_ms : IDCU_WATCHDOG_DEFAULT_TIMEOUT_MS;
    wd->max_restarts = max_restarts > 0 ? max_restarts : IDCU_WATCHDOG_MAX_RESTARTS;
    wd->restart_window_ms = restart_window_ms > 0 ? restart_window_ms : IDCU_WATCHDOG_RESTART_WINDOW_MS;
    wd->last_heartbeat = get_current_time_ms();
    wd->restart_window_start = get_current_time_ms();
    
    idcu_log_info("Watchdog initialized with timeout=%u ms, max_restarts=%u, window=%u ms",
                  wd->timeout_ms, wd->max_restarts, wd->restart_window_ms);
    
    return IDCU_ERR_OK;
}

void idcu_watchdog_destroy(idcu_Watchdog* wd) {
    if (!wd) {
        return;
    }
    
    idcu_watchdog_stop(wd);
    memset(wd, 0, sizeof(idcu_Watchdog));
}

int idcu_watchdog_start(idcu_Watchdog* wd) {
    if (!wd) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (wd->state == IDCU_WATCHDOG_STATE_RUNNING) {
        return IDCU_ERR_OK;
    }
    
    wd->should_exit = 0;
    wd->last_heartbeat = get_current_time_ms();
    
#ifdef _WIN32
    wd->thread_handle = CreateThread(NULL, 0, watchdog_thread, wd, 0, NULL);
    if (!wd->thread_handle) {
        idcu_log_error("Failed to create watchdog thread");
        wd->state = IDCU_WATCHDOG_STATE_ERROR;
        return IDCU_ERR_SYSTEM;
    }
#else
    if (pthread_create((pthread_t*)&wd->thread_handle, NULL, watchdog_thread, wd) != 0) {
        idcu_log_error("Failed to create watchdog thread");
        wd->state = IDCU_WATCHDOG_STATE_ERROR;
        return IDCU_ERR_SYSTEM;
    }
#endif
    
    wd->state = IDCU_WATCHDOG_STATE_RUNNING;
    idcu_log_info("Watchdog started");
    
    return IDCU_ERR_OK;
}

int idcu_watchdog_stop(idcu_Watchdog* wd) {
    if (!wd) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (wd->state != IDCU_WATCHDOG_STATE_RUNNING) {
        return IDCU_ERR_OK;
    }
    
    wd->should_exit = 1;
    
#ifdef _WIN32
    if (wd->thread_handle) {
        WaitForSingleObject(wd->thread_handle, INFINITE);
        CloseHandle(wd->thread_handle);
        wd->thread_handle = NULL;
    }
#else
    if (wd->thread_handle) {
        pthread_join(*(pthread_t*)&wd->thread_handle, NULL);
        wd->thread_handle = NULL;
    }
#endif
    
    wd->state = IDCU_WATCHDOG_STATE_STOPPED;
    idcu_log_info("Watchdog stopped");
    
    return IDCU_ERR_OK;
}

int idcu_watchdog_heartbeat(idcu_Watchdog* wd) {
    if (!wd) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    wd->last_heartbeat = get_current_time_ms();
    return IDCU_ERR_OK;
}

idcu_WatchdogState idcu_watchdog_get_state(idcu_Watchdog* wd) {
    if (!wd) {
        return IDCU_WATCHDOG_STATE_ERROR;
    }
    return wd->state;
}

uint32_t idcu_watchdog_get_restart_count(idcu_Watchdog* wd) {
    if (!wd) {
        return 0;
    }
    return wd->restart_count;
}

int idcu_watchdog_set_on_timeout(idcu_Watchdog* wd, idcu_WatchdogCallback callback, void* user_data) {
    (void)wd;
    g_on_timeout = callback;
    g_on_timeout_data = user_data;
    return IDCU_ERR_OK;
}

int idcu_watchdog_set_on_restart(idcu_Watchdog* wd, idcu_WatchdogCallback callback, void* user_data) {
    (void)wd;
    g_on_restart = callback;
    g_on_restart_data = user_data;
    return IDCU_ERR_OK;
}
