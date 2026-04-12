#include "idcu/os/os.h"
#include <stdlib.h>
#include <pthread.h>

struct idcu_mutex_t {
    pthread_mutex_t pmutex;
};

struct idcu_cond_t {
    pthread_cond_t pcond;
};

struct idcu_rwlock_t {
    pthread_rwlock_t prwlock;
};

idcu_mutex_t* idcu_mutex_create(void)
{
    idcu_mutex_t* mutex = (idcu_mutex_t*)malloc(sizeof(idcu_mutex_t));
    if (!mutex) {
        return NULL;
    }
    if (pthread_mutex_init(&mutex->pmutex, NULL) != 0) {
        free(mutex);
        return NULL;
    }
    return mutex;
}

void idcu_mutex_destroy(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return;
    }
    pthread_mutex_destroy(&mutex->pmutex);
    free(mutex);
}

int idcu_mutex_lock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    return pthread_mutex_lock(&mutex->pmutex) == 0 ? 0 : -1;
}

int idcu_mutex_unlock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    return pthread_mutex_unlock(&mutex->pmutex) == 0 ? 0 : -1;
}

int idcu_mutex_trylock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    return pthread_mutex_trylock(&mutex->pmutex) == 0 ? 0 : -1;
}

idcu_cond_t* idcu_cond_create(void)
{
    idcu_cond_t* cond = (idcu_cond_t*)malloc(sizeof(idcu_cond_t));
    if (!cond) {
        return NULL;
    }
    if (pthread_cond_init(&cond->pcond, NULL) != 0) {
        free(cond);
        return NULL;
    }
    return cond;
}

void idcu_cond_destroy(idcu_cond_t* cond)
{
    if (!cond) {
        return;
    }
    pthread_cond_destroy(&cond->pcond);
    free(cond);
}

int idcu_cond_wait(idcu_cond_t* cond, idcu_mutex_t* mutex)
{
    if (!cond || !mutex) {
        return -1;
    }
    return pthread_cond_wait(&cond->pcond, &mutex->pmutex) == 0 ? 0 : -1;
}

int idcu_cond_timedwait(idcu_cond_t* cond, idcu_mutex_t* mutex, uint32_t timeout_ms)
{
    if (!cond || !mutex) {
        return -1;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return pthread_cond_timedwait(&cond->pcond, &mutex->pmutex, &ts) == 0 ? 0 : -1;
}

int idcu_cond_signal(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    return pthread_cond_signal(&cond->pcond) == 0 ? 0 : -1;
}

int idcu_cond_broadcast(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    return pthread_cond_broadcast(&cond->pcond) == 0 ? 0 : -1;
}

idcu_rwlock_t* idcu_rwlock_create(void)
{
    idcu_rwlock_t* rwlock = (idcu_rwlock_t*)malloc(sizeof(idcu_rwlock_t));
    if (!rwlock) {
        return NULL;
    }
    if (pthread_rwlock_init(&rwlock->prwlock, NULL) != 0) {
        free(rwlock);
        return NULL;
    }
    return rwlock;
}

void idcu_rwlock_destroy(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return;
    }
    pthread_rwlock_destroy(&rwlock->prwlock);
    free(rwlock);
}

int idcu_rwlock_rdlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    return pthread_rwlock_rdlock(&rwlock->prwlock) == 0 ? 0 : -1;
}

int idcu_rwlock_wrlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    return pthread_rwlock_wrlock(&rwlock->prwlock) == 0 ? 0 : -1;
}

int idcu_rwlock_unlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    return pthread_rwlock_unlock(&rwlock->prwlock) == 0 ? 0 : -1;
}
