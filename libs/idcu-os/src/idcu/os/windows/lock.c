#include "idcu/os/os.h"
#include <stdlib.h>
#include <windows.h>

struct idcu_mutex_t {
    CRITICAL_SECTION cs;
};

struct idcu_cond_t {
    CONDITION_VARIABLE cv;
};

struct idcu_rwlock_t {
    SRWLOCK rwlock;
};

idcu_mutex_t* idcu_mutex_create(void)
{
    idcu_mutex_t* mutex = (idcu_mutex_t*)malloc(sizeof(idcu_mutex_t));
    if (!mutex) {
        return NULL;
    }
    InitializeCriticalSection(&mutex->cs);
    return mutex;
}

void idcu_mutex_destroy(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return;
    }
    DeleteCriticalSection(&mutex->cs);
    free(mutex);
}

int idcu_mutex_lock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    EnterCriticalSection(&mutex->cs);
    return 0;
}

int idcu_mutex_unlock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    LeaveCriticalSection(&mutex->cs);
    return 0;
}

int idcu_mutex_trylock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    return TryEnterCriticalSection(&mutex->cs) ? 0 : -1;
}

idcu_cond_t* idcu_cond_create(void)
{
    idcu_cond_t* cond = (idcu_cond_t*)malloc(sizeof(idcu_cond_t));
    if (!cond) {
        return NULL;
    }
    InitializeConditionVariable(&cond->cv);
    return cond;
}

void idcu_cond_destroy(idcu_cond_t* cond)
{
    if (!cond) {
        return;
    }
    free(cond);
}

int idcu_cond_wait(idcu_cond_t* cond, idcu_mutex_t* mutex)
{
    if (!cond || !mutex) {
        return -1;
    }
    SleepConditionVariableCS(&cond->cv, &mutex->cs, INFINITE);
    return 0;
}

int idcu_cond_timedwait(idcu_cond_t* cond, idcu_mutex_t* mutex, uint32_t timeout_ms)
{
    if (!cond || !mutex) {
        return -1;
    }
    return SleepConditionVariableCS(&cond->cv, &mutex->cs, timeout_ms) ? 0 : -1;
}

int idcu_cond_signal(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    WakeConditionVariable(&cond->cv);
    return 0;
}

int idcu_cond_broadcast(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    WakeAllConditionVariable(&cond->cv);
    return 0;
}

idcu_rwlock_t* idcu_rwlock_create(void)
{
    idcu_rwlock_t* rwlock = (idcu_rwlock_t*)malloc(sizeof(idcu_rwlock_t));
    if (!rwlock) {
        return NULL;
    }
    InitializeSRWLock(&rwlock->rwlock);
    return rwlock;
}

void idcu_rwlock_destroy(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return;
    }
    free(rwlock);
}

int idcu_rwlock_rdlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    AcquireSRWLockShared(&rwlock->rwlock);
    return 0;
}

int idcu_rwlock_wrlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    AcquireSRWLockExclusive(&rwlock->rwlock);
    return 0;
}

int idcu_rwlock_unlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    ReleaseSRWLockExclusive(&rwlock->rwlock);
    return 0;
}
