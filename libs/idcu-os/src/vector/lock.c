#include "idcu/os/os.h"
#include <stdlib.h>

struct idcu_mutex_t {
    int sem_id;
};

struct idcu_cond_t {
    int sem_id;
    int wait_count;
    idcu_mutex_t* wait_mutex;
};

struct idcu_rwlock_t {
    int read_sem;
    int write_sem;
    int read_count;
    idcu_mutex_t* count_mutex;
};

idcu_mutex_t* idcu_mutex_create(void)
{
    idcu_mutex_t* mutex = (idcu_mutex_t*)malloc(sizeof(idcu_mutex_t));
    if (!mutex) {
        return NULL;
    }
    mutex->sem_id = sem_create(1);
    if (mutex->sem_id < 0) {
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
    sem_destroy(mutex->sem_id);
    free(mutex);
}

int idcu_mutex_lock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    return sem_wait(mutex->sem_id) == 0 ? 0 : -1;
}

int idcu_mutex_unlock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    return sem_post(mutex->sem_id) == 0 ? 0 : -1;
}

int idcu_mutex_trylock(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return -1;
    }
    return sem_trywait(mutex->sem_id) == 0 ? 0 : -1;
}

idcu_cond_t* idcu_cond_create(void)
{
    idcu_cond_t* cond = (idcu_cond_t*)malloc(sizeof(idcu_cond_t));
    if (!cond) {
        return NULL;
    }
    cond->sem_id = sem_create(0);
    if (cond->sem_id < 0) {
        free(cond);
        return NULL;
    }
    cond->wait_count = 0;
    cond->wait_mutex = NULL;
    return cond;
}

void idcu_cond_destroy(idcu_cond_t* cond)
{
    if (!cond) {
        return;
    }
    sem_destroy(cond->sem_id);
    free(cond);
}

int idcu_cond_wait(idcu_cond_t* cond, idcu_mutex_t* mutex)
{
    if (!cond || !mutex) {
        return -1;
    }
    cond->wait_count++;
    if (idcu_mutex_unlock(mutex) != 0) {
        cond->wait_count--;
        return -1;
    }
    if (sem_wait(cond->sem_id) != 0) {
        cond->wait_count--;
        idcu_mutex_lock(mutex);
        return -1;
    }
    cond->wait_count--;
    return idcu_mutex_lock(mutex);
}

int idcu_cond_timedwait(idcu_cond_t* cond, idcu_mutex_t* mutex, uint32_t timeout_ms)
{
    (void)cond;
    (void)mutex;
    (void)timeout_ms;
    return -1;
}

int idcu_cond_signal(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    if (cond->wait_count > 0) {
        return sem_post(cond->sem_id) == 0 ? 0 : -1;
    }
    return 0;
}

int idcu_cond_broadcast(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    for (int i = 0; i < cond->wait_count; i++) {
        if (sem_post(cond->sem_id) != 0) {
            return -1;
        }
    }
    return 0;
}

idcu_rwlock_t* idcu_rwlock_create(void)
{
    idcu_rwlock_t* rwlock = (idcu_rwlock_t*)malloc(sizeof(idcu_rwlock_t));
    if (!rwlock) {
        return NULL;
    }
    rwlock->read_sem = sem_create(1);
    if (rwlock->read_sem < 0) {
        free(rwlock);
        return NULL;
    }
    rwlock->write_sem = sem_create(1);
    if (rwlock->write_sem < 0) {
        sem_destroy(rwlock->read_sem);
        free(rwlock);
        return NULL;
    }
    rwlock->count_mutex = idcu_mutex_create();
    if (!rwlock->count_mutex) {
        sem_destroy(rwlock->read_sem);
        sem_destroy(rwlock->write_sem);
        free(rwlock);
        return NULL;
    }
    rwlock->read_count = 0;
    return rwlock;
}

void idcu_rwlock_destroy(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return;
    }
    idcu_mutex_destroy(rwlock->count_mutex);
    sem_destroy(rwlock->read_sem);
    sem_destroy(rwlock->write_sem);
    free(rwlock);
}

int idcu_rwlock_rdlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    if (idcu_mutex_lock(rwlock->count_mutex) != 0) {
        return -1;
    }
    rwlock->read_count++;
    if (rwlock->read_count == 1) {
        if (sem_wait(rwlock->write_sem) != 0) {
            rwlock->read_count--;
            idcu_mutex_unlock(rwlock->count_mutex);
            return -1;
        }
    }
    idcu_mutex_unlock(rwlock->count_mutex);
    return 0;
}

int idcu_rwlock_wrlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    return sem_wait(rwlock->write_sem) == 0 ? 0 : -1;
}

int idcu_rwlock_unlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    if (idcu_mutex_lock(rwlock->count_mutex) != 0) {
        return -1;
    }
    if (rwlock->read_count > 0) {
        rwlock->read_count--;
        if (rwlock->read_count == 0) {
            sem_post(rwlock->write_sem);
        }
    } else {
        sem_post(rwlock->write_sem);
    }
    idcu_mutex_unlock(rwlock->count_mutex);
    return 0;
}
