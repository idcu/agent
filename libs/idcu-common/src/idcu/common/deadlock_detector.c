#include "idcu/common/deadlock_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#endif

static void log_message(idcu_DeadlockDetector *detector, idcu_DeadlockLogLevel level,
                        const char *message) {
    if (detector && detector->log_callback) {
        detector->log_callback(level, message, detector->log_callback_data);
    }
}

static idcu_ThreadId get_current_thread_id(void) {
#ifdef _WIN32
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

static int thread_ids_equal(idcu_ThreadId t1, idcu_ThreadId t2) {
#ifdef _WIN32
    return t1 == t2;
#else
    return pthread_equal(t1, t2);
#endif
}

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
static DWORD WINAPI deadlock_detector_thread(LPVOID param)
#else
static void *deadlock_detector_thread(void *param)
#endif
{
    idcu_DeadlockDetector *detector = (idcu_DeadlockDetector *)param;

    log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_INFO, "Deadlock detector thread started");

    while (!detector->should_exit) {
        if (detector->enabled) {
            idcu_deadlock_detector_check(detector);
        }
        sleep_ms(IDCU_DEADLOCK_CHECK_INTERVAL_MS);
    }

    log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_INFO, "Deadlock detector thread stopped");

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int idcu_deadlock_detector_init(idcu_DeadlockDetector *detector) {
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    memset(detector, 0, sizeof(idcu_DeadlockDetector));
    int ret = idcu_mutex_init(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    detector->enabled = 0;
    detector->should_exit = 0;
    log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_INFO, "Deadlock detector initialized");

    return IDCU_ERR_SUCCESS;
}

void idcu_deadlock_detector_destroy(idcu_DeadlockDetector *detector) {
    if (!detector) {
        return;
    }

    idcu_deadlock_detector_stop(detector);
    idcu_mutex_destroy(&detector->detector_lock);
    memset(detector, 0, sizeof(idcu_DeadlockDetector));
}

int idcu_deadlock_detector_start(idcu_DeadlockDetector *detector) {
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (detector->enabled) {
        return IDCU_ERR_OK;
    }

    detector->should_exit = 0;
    detector->enabled = 1;

#ifdef _WIN32
    detector->thread_handle = CreateThread(NULL, 0, deadlock_detector_thread, detector, 0, NULL);
    if (!detector->thread_handle) {
        log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_ERROR,
                    "Failed to create deadlock detector thread");
        detector->enabled = 0;
        return IDCU_ERR_GENERAL;
    }
#else
    if (pthread_create((pthread_t *)&detector->thread_handle, NULL, deadlock_detector_thread,
                       detector) != 0) {
        log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_ERROR,
                    "Failed to create deadlock detector thread");
        detector->enabled = 0;
        return IDCU_ERR_GENERAL;
    }
#endif

    log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_INFO, "Deadlock detector started");
    return IDCU_ERR_OK;
}

int idcu_deadlock_detector_stop(idcu_DeadlockDetector *detector) {
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    if (!detector->enabled) {
        return IDCU_ERR_OK;
    }

    detector->should_exit = 1;
    detector->enabled = 0;

#ifdef _WIN32
    if (detector->thread_handle) {
        WaitForSingleObject(detector->thread_handle, INFINITE);
        CloseHandle(detector->thread_handle);
        detector->thread_handle = NULL;
    }
#else
    if (detector->thread_handle) {
        pthread_join(*(pthread_t *)&detector->thread_handle, NULL);
        detector->thread_handle = NULL;
    }
#endif

    log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_INFO, "Deadlock detector stopped");
    return IDCU_ERR_OK;
}

int idcu_deadlock_detector_register_lock(idcu_DeadlockDetector *detector, idcu_Mutex *lock,
                                         const char *name) {
    if (!detector || !lock) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_LOCKS; i++) {
        if (!detector->locks[i].in_use) {
            detector->locks[i].lock = lock;
            detector->locks[i].lock_name = name;
            detector->locks[i].in_use = 1;
            idcu_mutex_unlock(&detector->detector_lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
    log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_ERROR, "No available slot to register lock");
    return IDCU_ERR_NO_MEMORY;
}

int idcu_deadlock_detector_unregister_lock(idcu_DeadlockDetector *detector, idcu_Mutex *lock) {
    if (!detector || !lock) {
        return IDCU_ERR_INVALID_PARAM;
    }

    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_LOCKS; i++) {
        if (detector->locks[i].in_use && detector->locks[i].lock == lock) {
            memset(&detector->locks[i], 0, sizeof(idcu_LockInfo));
            idcu_mutex_unlock(&detector->detector_lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_deadlock_detector_register_thread(idcu_DeadlockDetector *detector, const char *name) {
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ThreadId tid = get_current_thread_id();
    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_THREADS; i++) {
        if (!detector->threads[i].is_waiting && !detector->threads[i].thread_name) {
            detector->threads[i].thread_id = tid;
            detector->threads[i].thread_name = name;
            detector->threads[i].is_waiting = 0;
            idcu_mutex_unlock(&detector->detector_lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
    log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_ERROR, "No available slot to register thread");
    return IDCU_ERR_NO_MEMORY;
}

int idcu_deadlock_detector_unregister_thread(idcu_DeadlockDetector *detector) {
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_ThreadId tid = get_current_thread_id();
    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_THREADS; i++) {
        if (thread_ids_equal(detector->threads[i].thread_id, tid)) {
            memset(&detector->threads[i], 0, sizeof(idcu_ThreadInfo));
            idcu_mutex_unlock(&detector->detector_lock);
            return IDCU_ERR_SUCCESS;
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
    return IDCU_ERR_NOT_FOUND;
}

int idcu_deadlock_detector_before_lock(idcu_DeadlockDetector *detector, idcu_Mutex *lock) {
    if (!detector || !lock || !detector->enabled) {
        return IDCU_ERR_OK;
    }

    idcu_ThreadId tid = get_current_thread_id();
    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_THREADS; i++) {
        if (thread_ids_equal(detector->threads[i].thread_id, tid)) {
            detector->threads[i].waiting_for_lock = lock;
            detector->threads[i].wait_start_time = get_current_time_ms();
            detector->threads[i].is_waiting = 1;
            break;
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
    return IDCU_ERR_OK;
}

int idcu_deadlock_detector_after_lock(idcu_DeadlockDetector *detector, idcu_Mutex *lock) {
    if (!detector || !lock || !detector->enabled) {
        return IDCU_ERR_OK;
    }

    idcu_ThreadId tid = get_current_thread_id();
    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_LOCKS; i++) {
        if (detector->locks[i].in_use && detector->locks[i].lock == lock) {
            detector->locks[i].owner_thread = tid;
            detector->locks[i].acquire_time = get_current_time_ms();
            break;
        }
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_THREADS; i++) {
        if (thread_ids_equal(detector->threads[i].thread_id, tid)) {
            detector->threads[i].waiting_for_lock = NULL;
            detector->threads[i].is_waiting = 0;
            break;
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
    return IDCU_ERR_OK;
}

int idcu_deadlock_detector_before_unlock(idcu_DeadlockDetector *detector, idcu_Mutex *lock) {
    if (!detector || !lock || !detector->enabled) {
        return IDCU_ERR_OK;
    }

    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return ret;
    }

    for (int i = 0; i < IDCU_DEADLOCK_MAX_LOCKS; i++) {
        if (detector->locks[i].in_use && detector->locks[i].lock == lock) {
            memset(&detector->locks[i].owner_thread, 0, sizeof(idcu_ThreadId));
            detector->locks[i].acquire_time = 0;
            break;
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
    return IDCU_ERR_OK;
}

int idcu_deadlock_detector_set_callback(idcu_DeadlockDetector *detector,
                                        idcu_DeadlockCallback callback, void *user_data) {
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    detector->deadlock_callback = callback;
    detector->callback_data = user_data;
    return IDCU_ERR_OK;
}

int idcu_deadlock_detector_set_log_callback(idcu_DeadlockDetector *detector,
                                            idcu_DeadlockLogCallback callback, void *user_data) {
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }

    detector->log_callback = callback;
    detector->log_callback_data = user_data;
    return IDCU_ERR_OK;
}

void idcu_deadlock_detector_check(idcu_DeadlockDetector *detector) {
    if (!detector || !detector->enabled) {
        return;
    }

    int ret = idcu_mutex_lock(&detector->detector_lock);
    if (ret != IDCU_ERR_SUCCESS) {
        return;
    }

    uint64_t current_time = get_current_time_ms();

    for (int i = 0; i < IDCU_DEADLOCK_MAX_THREADS; i++) {
        if (!detector->threads[i].is_waiting) {
            continue;
        }

        uint64_t wait_time = current_time - detector->threads[i].wait_start_time;
        if (wait_time > IDCU_DEADLOCK_MAX_WAIT_MS) {
            idcu_Mutex *waiting_for = detector->threads[i].waiting_for_lock;
            const char *thread1_name =
                detector->threads[i].thread_name ? detector->threads[i].thread_name : "unknown";

            for (int j = 0; j < IDCU_DEADLOCK_MAX_LOCKS; j++) {
                if (detector->locks[j].in_use && detector->locks[j].lock == waiting_for) {
                    const char *lock1_name =
                        detector->locks[j].lock_name ? detector->locks[j].lock_name : "unknown";

                    for (int k = 0; k < IDCU_DEADLOCK_MAX_THREADS; k++) {
                        if (k == i)
                            continue;

                        if (thread_ids_equal(detector->threads[k].thread_id,
                                             detector->locks[j].owner_thread) &&
                            detector->threads[k].is_waiting) {

                            for (int l = 0; l < IDCU_DEADLOCK_MAX_LOCKS; l++) {
                                if (detector->locks[l].in_use &&
                                    detector->locks[l].lock ==
                                        detector->threads[k].waiting_for_lock) {
                                    if (thread_ids_equal(detector->locks[l].owner_thread,
                                                         detector->threads[i].thread_id)) {
                                        const char *thread2_name =
                                            detector->threads[k].thread_name
                                                ? detector->threads[k].thread_name
                                                : "unknown";
                                        const char *lock2_name = detector->locks[l].lock_name
                                                                     ? detector->locks[l].lock_name
                                                                     : "unknown";

                                        char log_buffer[512];
                                        snprintf(
                                            log_buffer, sizeof(log_buffer),
                                            "Potential deadlock detected: Thread '%s' waiting for "
                                            "lock '%s', "
                                            "held by thread '%s' which is waiting for lock '%s'",
                                            thread1_name, lock1_name, thread2_name, lock2_name);
                                        log_message(detector, IDCU_DEADLOCK_LOG_LEVEL_ERROR,
                                                    log_buffer);

                                        if (detector->deadlock_callback) {
                                            detector->deadlock_callback(thread1_name, thread2_name,
                                                                        lock1_name, lock2_name,
                                                                        detector->callback_data);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    idcu_mutex_unlock(&detector->detector_lock);
}
