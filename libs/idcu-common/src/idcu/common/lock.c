#include "idcu/common/lock.h"

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
#endif
    return IDCU_ERR_OK;
#else
    int ret = pthread_mutex_init(&mutex->pmutex, NULL);
#ifdef IDCU_DEBUG
    mutex->owner_thread = (pthread_t)0;
    mutex->lock_count = 0;
    mutex->total_locks = 0;
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
    EnterCriticalSection(&mutex->cs);
#ifdef IDCU_DEBUG
    mutex->owner_thread = GetCurrentThreadId();
    mutex->lock_count++;
    mutex->total_locks++;
#endif
    return IDCU_ERR_OK;
#else
    int ret = pthread_mutex_lock(&mutex->pmutex);
#ifdef IDCU_DEBUG
    if (ret == 0) {
        mutex->owner_thread = pthread_self();
        mutex->lock_count++;
        mutex->total_locks++;
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
#endif
        return IDCU_ERR_OK;
    } else if (ret == EBUSY) {
        return IDCU_ERR_BUSY;
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
