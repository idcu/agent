#include "idcu/common/lock.h"

int idcu_mutex_init(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    InitializeCriticalSection(mutex);
    return IDCU_ERR_OK;
#else
    int ret = pthread_mutex_init(mutex, NULL);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_GENERAL;
#endif
}

void idcu_mutex_destroy(idcu_Mutex* mutex)
{
    if (!mutex)
        return;

#ifdef _WIN32
    DeleteCriticalSection(mutex);
#else
    pthread_mutex_destroy(mutex);
#endif
}

int idcu_mutex_lock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    EnterCriticalSection(mutex);
    return IDCU_ERR_OK;
#else
    int ret = pthread_mutex_lock(mutex);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_LOCK_FAILED;
#endif
}

int idcu_mutex_unlock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    LeaveCriticalSection(mutex);
    return IDCU_ERR_OK;
#else
    int ret = pthread_mutex_unlock(mutex);
    return ret == 0 ? IDCU_ERR_OK : IDCU_ERR_UNLOCK_FAILED;
#endif
}

int idcu_mutex_trylock(idcu_Mutex* mutex)
{
    if (!mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    if (TryEnterCriticalSection(mutex)) {
        return IDCU_ERR_OK;
    }
    return IDCU_ERR_BUSY;
#else
    int ret = pthread_mutex_trylock(mutex);
    if (ret == 0) {
        return IDCU_ERR_OK;
    } else if (ret == EBUSY) {
        return IDCU_ERR_BUSY;
    }
    return IDCU_ERR_LOCK_FAILED;
#endif
}

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
