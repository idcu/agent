#include "idcu/common/lock.h"

int idcu_mutex_init(idcu_Mutex *lock) {
    if (!lock) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    InitializeCriticalSection(&lock->cs);
#else
    if (pthread_mutex_init(&lock->mutex, NULL) != 0) {
        return IDCU_ERR_GENERAL;
    }
#endif
    lock->initialized = 1;
    return IDCU_ERR_SUCCESS;
}

void idcu_mutex_destroy(idcu_Mutex *lock) {
    if (!lock || !lock->initialized) {
        return;
    }

#ifdef _WIN32
    DeleteCriticalSection(&lock->cs);
#else
    pthread_mutex_destroy(&lock->mutex);
#endif
    lock->initialized = 0;
}

int idcu_mutex_lock(idcu_Mutex *lock) {
    if (!lock || !lock->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    EnterCriticalSection(&lock->cs);
#else
    if (pthread_mutex_lock(&lock->mutex) != 0) {
        return IDCU_ERR_LOCK_FAILED;
    }
#endif
    return IDCU_ERR_SUCCESS;
}

int idcu_mutex_unlock(idcu_Mutex *lock) {
    if (!lock || !lock->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    LeaveCriticalSection(&lock->cs);
#else
    if (pthread_mutex_unlock(&lock->mutex) != 0) {
        return IDCU_ERR_UNLOCK_FAILED;
    }
#endif
    return IDCU_ERR_SUCCESS;
}

int idcu_rwlock_init(idcu_RwLock *lock) {
    if (!lock) {
        return IDCU_ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    InitializeSRWLock(&lock->rwlock);
#else
    if (pthread_rwlock_init(&lock->rwlock, NULL) != 0) {
        return IDCU_ERR_GENERAL;
    }
#endif
    lock->initialized = 1;
    return IDCU_ERR_SUCCESS;
}

void idcu_rwlock_destroy(idcu_RwLock *lock) {
    if (!lock || !lock->initialized) {
        return;
    }

#ifndef _WIN32
    pthread_rwlock_destroy(&lock->rwlock);
#endif
    lock->initialized = 0;
}

int idcu_rwlock_rdlock(idcu_RwLock *lock) {
    if (!lock || !lock->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    AcquireSRWLockShared(&lock->rwlock);
#else
    if (pthread_rwlock_rdlock(&lock->rwlock) != 0) {
        return IDCU_ERR_LOCK_FAILED;
    }
#endif
    return IDCU_ERR_SUCCESS;
}

int idcu_rwlock_wrlock(idcu_RwLock *lock) {
    if (!lock || !lock->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    AcquireSRWLockExclusive(&lock->rwlock);
#else
    if (pthread_rwlock_wrlock(&lock->rwlock) != 0) {
        return IDCU_ERR_LOCK_FAILED;
    }
#endif
    return IDCU_ERR_SUCCESS;
}

int idcu_rwlock_unlock(idcu_RwLock *lock) {
    if (!lock || !lock->initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    ReleaseSRWLockExclusive(&lock->rwlock);
#else
    if (pthread_rwlock_unlock(&lock->rwlock) != 0) {
        return IDCU_ERR_UNLOCK_FAILED;
    }
#endif
    return IDCU_ERR_SUCCESS;
}
