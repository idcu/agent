#include "common/lock.h"

int mutex_init(Mutex* lock)
{
    if (!lock) {
        return ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    InitializeCriticalSection(&lock->cs);
#else
    if (pthread_mutex_init(&lock->mutex, NULL) != 0) {
        return ERR_GENERAL;
    }
#endif
    lock->initialized = 1;
    return ERR_SUCCESS;
}

void mutex_destroy(Mutex* lock)
{
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

int mutex_lock(Mutex* lock)
{
    if (!lock || !lock->initialized) {
        return ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    EnterCriticalSection(&lock->cs);
#else
    if (pthread_mutex_lock(&lock->mutex) != 0) {
        return ERR_LOCK_FAILED;
    }
#endif
    return ERR_SUCCESS;
}

int mutex_unlock(Mutex* lock)
{
    if (!lock || !lock->initialized) {
        return ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    LeaveCriticalSection(&lock->cs);
#else
    if (pthread_mutex_unlock(&lock->mutex) != 0) {
        return ERR_UNLOCK_FAILED;
    }
#endif
    return ERR_SUCCESS;
}

int rwlock_init(RwLock* lock)
{
    if (!lock) {
        return ERR_INVALID_PARAM;
    }

#ifdef _WIN32
    InitializeSRWLock(&lock->rwlock);
#else
    if (pthread_rwlock_init(&lock->rwlock, NULL) != 0) {
        return ERR_GENERAL;
    }
#endif
    lock->initialized = 1;
    return ERR_SUCCESS;
}

void rwlock_destroy(RwLock* lock)
{
    if (!lock || !lock->initialized) {
        return;
    }

#ifndef _WIN32
    pthread_rwlock_destroy(&lock->rwlock);
#endif
    lock->initialized = 0;
}

int rwlock_rdlock(RwLock* lock)
{
    if (!lock || !lock->initialized) {
        return ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    AcquireSRWLockShared(&lock->rwlock);
#else
    if (pthread_rwlock_rdlock(&lock->rwlock) != 0) {
        return ERR_LOCK_FAILED;
    }
#endif
    return ERR_SUCCESS;
}

int rwlock_wrlock(RwLock* lock)
{
    if (!lock || !lock->initialized) {
        return ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    AcquireSRWLockExclusive(&lock->rwlock);
#else
    if (pthread_rwlock_wrlock(&lock->rwlock) != 0) {
        return ERR_LOCK_FAILED;
    }
#endif
    return ERR_SUCCESS;
}

int rwlock_unlock(RwLock* lock)
{
    if (!lock || !lock->initialized) {
        return ERR_NOT_INITIALIZED;
    }

#ifdef _WIN32
    ReleaseSRWLockExclusive(&lock->rwlock);
#else
    if (pthread_rwlock_unlock(&lock->rwlock) != 0) {
        return ERR_UNLOCK_FAILED;
    }
#endif
    return ERR_SUCCESS;
}
