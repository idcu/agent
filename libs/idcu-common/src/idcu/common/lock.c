#include "idcu/common/lock.h"

#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

static uint64_t idcu_get_current_time_ms(void)
{
#ifdef _WIN32
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    return (uint64_t)timebuffer.time * 1000 + (uint64_t)timebuffer.millitm;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

int idcu_mutex_init(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    InitializeCriticalSection(&mutex->cs);
#ifdef IDCU_DEBUG
    mutex->owner_thread = 0;
    mutex->lock_count = 0;
    mutex->total_locks = 0;
    mutex->lock_acquire_time = 0;
    mutex->total_hold_time = 0;
    mutex->max_hold_time = 0;
    mutex->lock_wait_start = 0;
    mutex->total_wait_time = 0;
    mutex->max_wait_time = 0;
#endif
    return IDCU_ERR_OK;
#else
    int ret = pthread_mutex_init(&mutex->pmutex, NULL);
#ifdef IDCU_DEBUG
    mutex->owner_thread = (pthread_t)0;
    mutex->lock_count = 0;
    mutex->total_locks = 0;
    mutex->lock_acquire_time = 0;
    mutex->total_hold_time = 0;
    mutex->max_hold_time = 0;
    mutex->lock_wait_start = 0;
    mutex->total_wait_time = 0;
    mutex->max_wait_time = 0;
#endif
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_GENERAL;
#endif
}

void idcu_mutex_destroy(idcu_Mutex* mutex)
{
    if (!mutex)
        return;

#ifdef _WIN32
    DeleteCriticalSection(&mutex->cs);
#else
    pthread_mutex_destroy(&mutex->pmutex);
#endif
}

int idcu_mutex_lock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
#ifdef IDCU_DEBUG
    mutex->lock_wait_start = idcu_get_current_time_ms();
#endif
    EnterCriticalSection(&mutex->cs);
#ifdef IDCU_DEBUG
    uint64_t wait_time = idcu_get_current_time_ms() - mutex->lock_wait_start;
    mutex->total_wait_time += wait_time;
    if (wait_time > mutex->max_wait_time) {
        mutex->max_wait_time = wait_time;
    }
    mutex->owner_thread = GetCurrentThreadId();
    mutex->lock_count++;
    mutex->total_locks++;
    mutex->lock_acquire_time = idcu_get_current_time_ms();
#endif
    return IDCU_ERR_OK;
#else
#ifdef IDCU_DEBUG
    mutex->lock_wait_start = idcu_get_current_time_ms();
#endif
    int ret = pthread_mutex_lock(&mutex->pmutex);
#ifdef IDCU_DEBUG
    if (ret == 0) {
        uint64_t wait_time = idcu_get_current_time_ms() - mutex->lock_wait_start;
        mutex->total_wait_time += wait_time;
        if (wait_time > mutex->max_wait_time) {
            mutex->max_wait_time = wait_time;
        }
        mutex->owner_thread = pthread_self();
        mutex->lock_count++;
        mutex->total_locks++;
        mutex->lock_acquire_time = idcu_get_current_time_ms();
    }
#endif
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_LOCK_FAILED;
#endif
}

int idcu_mutex_unlock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
#ifdef IDCU_DEBUG
    if (mutex->lock_count > 0) {
        uint64_t hold_time = idcu_get_current_time_ms() - mutex->lock_acquire_time;
        mutex->total_hold_time += hold_time;
        if (hold_time > mutex->max_hold_time) {
            mutex->max_hold_time = hold_time;
        }
        mutex->lock_count--;
        if (mutex->lock_count == 0) {
            mutex->owner_thread = 0;
        }
    }
#endif
    LeaveCriticalSection(&mutex->cs);
    return IDCU_ERR_OK;
#else
#ifdef IDCU_DEBUG
    if (mutex->lock_count > 0) {
        uint64_t hold_time = idcu_get_current_time_ms() - mutex->lock_acquire_time;
        mutex->total_hold_time += hold_time;
        if (hold_time > mutex->max_hold_time) {
            mutex->max_hold_time = hold_time;
        }
        mutex->lock_count--;
        if (mutex->lock_count == 0) {
            mutex->owner_thread = (pthread_t)0;
        }
    }
#endif
    int ret = pthread_mutex_unlock(&mutex->pmutex);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_UNLOCK_FAILED;
#endif
}

int idcu_mutex_trylock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    if (TryEnterCriticalSection(&mutex->cs)) {
#ifdef IDCU_DEBUG
        mutex->owner_thread = GetCurrentThreadId();
        mutex->lock_count++;
        mutex->total_locks++;
        mutex->lock_acquire_time = idcu_get_current_time_ms();
#endif
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_BUSY;
#else
    int ret = pthread_mutex_trylock(&mutex->pmutex);
    if (ret == 0) {
#ifdef IDCU_DEBUG
        mutex->owner_thread = pthread_self();
        mutex->lock_count++;
        mutex->total_locks++;
        mutex->lock_acquire_time = idcu_get_current_time_ms();
#endif
        return IDCU_ERR_OK;
    } else if (ret == EBUSY) {
        return IDCU_ERR_BUSY;
    }
    return IDCU_ERR_LOCK_FAILED;
#endif
}

int idcu_mutex_timedlock(idcu_Mutex* mutex, uint32_t timeout_ms)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    uint64_t start = idcu_get_current_time_ms();
    uint64_t elapsed = 0;
#ifdef IDCU_DEBUG
    mutex->lock_wait_start = start;
#endif
    while (elapsed < timeout_ms) {
        if (TryEnterCriticalSection(&mutex->cs)) {
#ifdef IDCU_DEBUG
            uint64_t wait_time = idcu_get_current_time_ms() - mutex->lock_wait_start;
            mutex->total_wait_time += wait_time;
            if (wait_time > mutex->max_wait_time) {
                mutex->max_wait_time = wait_time;
            }
            mutex->owner_thread = GetCurrentThreadId();
            mutex->lock_count++;
            mutex->total_locks++;
            mutex->lock_acquire_time = idcu_get_current_time_ms();
#endif
            return IDCU_ERR_OK;
        }
        Sleep(1);
        elapsed = idcu_get_current_time_ms() - start;
    }
    return IDCU_ERR_TIMEOUT;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
#ifdef IDCU_DEBUG
    mutex->lock_wait_start = idcu_get_current_time_ms();
#endif
    int ret = pthread_mutex_timedlock(&mutex->pmutex, &ts);
#ifdef IDCU_DEBUG
    if (ret == 0) {
        uint64_t wait_time = idcu_get_current_time_ms() - mutex->lock_wait_start;
        mutex->total_wait_time += wait_time;
        if (wait_time > mutex->max_wait_time) {
            mutex->max_wait_time = wait_time;
        }
        mutex->owner_thread = pthread_self();
        mutex->lock_count++;
        mutex->total_locks++;
        mutex->lock_acquire_time = idcu_get_current_time_ms();
    }
#endif
    if (ret == 0) {
        return IDCU_ERR_OK;
    } else if (ret == ETIMEDOUT) {
        return IDCU_ERR_TIMEOUT;
    }
    return IDCU_ERR_LOCK_FAILED;
#endif
}

#ifdef IDCU_DEBUG
uint64_t idcu_mutex_get_lock_count(idcu_Mutex* mutex)
{
    if (!mutex) {
        return 0;
    }
    return mutex->lock_count;
}

uint64_t idcu_mutex_get_total_locks(idcu_Mutex* mutex)
{
    if (!mutex) {
        return 0;
    }
    return mutex->total_locks;
}

int idcu_mutex_is_held_by_current_thread(idcu_Mutex* mutex)
{
    if (!mutex) {
        return 0;
    }
#ifdef _WIN32
    return mutex->owner_thread == GetCurrentThreadId();
#else
    return pthread_equal(mutex->owner_thread, pthread_self());
#endif
}

uint64_t idcu_mutex_get_total_hold_time(idcu_Mutex* mutex)
{
    if (!mutex) {
        return 0;
    }
    return mutex->total_hold_time;
}

uint64_t idcu_mutex_get_max_hold_time(idcu_Mutex* mutex)
{
    if (!mutex) {
        return 0;
    }
    return mutex->max_hold_time;
}

uint64_t idcu_mutex_get_total_wait_time(idcu_Mutex* mutex)
{
    if (!mutex) {
        return 0;
    }
    return mutex->total_wait_time;
}

uint64_t idcu_mutex_get_max_wait_time(idcu_Mutex* mutex)
{
    if (!mutex) {
        return 0;
    }
    return mutex->max_wait_time;
}

uint64_t idcu_mutex_get_current_wait_time(idcu_Mutex* mutex)
{
    if (!mutex || mutex->lock_wait_start == 0) {
        return 0;
    }
    return idcu_get_current_time_ms() - mutex->lock_wait_start;
}

void idcu_mutex_print_stats(idcu_Mutex* mutex, const char* name)
{
    if (!mutex) {
        return;
    }
    printf("=== Mutex Statistics: %s ===\n", name ? name : "unnamed");
    printf("  Total locks: %llu\n", (unsigned long long)mutex->total_locks);
    printf("  Current lock count: %llu\n", (unsigned long long)mutex->lock_count);
    printf("  Total hold time: %llu ms\n", (unsigned long long)mutex->total_hold_time);
    printf("  Max hold time: %llu ms\n", (unsigned long long)mutex->max_hold_time);
    printf("  Total wait time: %llu ms\n", (unsigned long long)mutex->total_wait_time);
    printf("  Max wait time: %llu ms\n", (unsigned long long)mutex->max_wait_time);
    printf("==============================\n");
}
#endif

int idcu_lock_guard_init(idcu_LockGuard* guard, idcu_Mutex* mutex)
{
    if (!guard || !mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }
    guard->mutex = mutex;
    return idcu_mutex_lock(mutex);
}

void idcu_lock_guard_destroy(idcu_LockGuard* guard)
{
    if (!guard || !guard->mutex)
        return;
    idcu_mutex_unlock(guard->mutex);
}

int idcu_cond_init(idcu_Condition* cond)
{
    if (!cond) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    InitializeConditionVariable(&cond->cv);
    return IDCU_ERR_OK;
#else
    int ret = pthread_cond_init(&cond->pcond, NULL);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_GENERAL;
#endif
}

void idcu_cond_destroy(idcu_Condition* cond)
{
    if (!cond)
        return;

#ifdef _WIN32
    // Windows CONDITION_VARIABLE doesn't need explicit destruction
    (void)cond;
#else
    pthread_cond_destroy(&cond->pcond);
#endif
}

int idcu_cond_wait(idcu_Condition* cond, idcu_Mutex* mutex)
{
    if (!cond || !mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    if (!SleepConditionVariableCS(&cond->cv, &mutex->cs, INFINITE)) {
        return IDCU_ERR_GENERAL;
    }
    return IDCU_ERR_OK;
#else
    int ret = pthread_cond_wait(&cond->pcond, &mutex->pmutex);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_GENERAL;
#endif
}

int idcu_cond_signal(idcu_Condition* cond)
{
    if (!cond) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    WakeConditionVariable(&cond->cv);
    return IDCU_ERR_OK;
#else
    int ret = pthread_cond_signal(&cond->pcond);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_GENERAL;
#endif
}

int idcu_cond_broadcast(idcu_Condition* cond)
{
    if (!cond) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    WakeAllConditionVariable(&cond->cv);
    return IDCU_ERR_OK;
#else
    int ret = pthread_cond_broadcast(&cond->pcond);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_GENERAL;
#endif
}
