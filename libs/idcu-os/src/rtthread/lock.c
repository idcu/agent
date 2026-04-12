#include "idcu/os/os.h"
#include <rtthread.h>
#include <stdlib.h>

struct idcu_mutex_t {
    rt_mutex_t mutex;
};

struct idcu_cond_t {
    rt_sem_t sem;
    int waiters;
    rt_mutex_t waiter_lock;
};

struct idcu_rwlock_t {
    rt_mutex_t mutex;
    rt_sem_t read_sem;
    rt_sem_t write_sem;
    int readers;
    int writers;
    int write_waiters;
};

idcu_mutex_t* idcu_mutex_create(void)
{
    idcu_mutex_t* mutex = (idcu_mutex_t*)rt_malloc(sizeof(idcu_mutex_t));
    if (!mutex) {
        return NULL;
    }
    mutex->mutex = rt_mutex_create("idcu_mutex", RT_IPC_FLAG_PRIO);
    if (!mutex->mutex) {
        rt_free(mutex);
        return NULL;
    }
    return mutex;
}

void idcu_mutex_destroy(idcu_mutex_t* mutex)
{
    if (!mutex) {
        return;
    }
    if (mutex->mutex) {
        rt_mutex_delete(mutex->mutex);
    }
    rt_free(mutex);
}

int idcu_mutex_lock(idcu_mutex_t* mutex)
{
    if (!mutex || !mutex->mutex) {
        return -1;
    }
    return rt_mutex_take(mutex->mutex, RT_WAITING_FOREVER) == RT_EOK ? 0 : -1;
}

int idcu_mutex_unlock(idcu_mutex_t* mutex)
{
    if (!mutex || !mutex->mutex) {
        return -1;
    }
    return rt_mutex_release(mutex->mutex) == RT_EOK ? 0 : -1;
}

int idcu_mutex_trylock(idcu_mutex_t* mutex)
{
    if (!mutex || !mutex->mutex) {
        return -1;
    }
    return rt_mutex_take(mutex->mutex, 0) == RT_EOK ? 0 : -1;
}

idcu_cond_t* idcu_cond_create(void)
{
    idcu_cond_t* cond = (idcu_cond_t*)rt_malloc(sizeof(idcu_cond_t));
    if (!cond) {
        return NULL;
    }
    cond->sem = rt_sem_create("idcu_cond", 0, RT_IPC_FLAG_PRIO);
    cond->waiter_lock = rt_mutex_create("idcu_cond_lock", RT_IPC_FLAG_PRIO);
    cond->waiters = 0;
    if (!cond->sem || !cond->waiter_lock) {
        if (cond->sem) rt_sem_delete(cond->sem);
        if (cond->waiter_lock) rt_mutex_delete(cond->waiter_lock);
        rt_free(cond);
        return NULL;
    }
    return cond;
}

void idcu_cond_destroy(idcu_cond_t* cond)
{
    if (!cond) {
        return;
    }
    if (cond->sem) rt_sem_delete(cond->sem);
    if (cond->waiter_lock) rt_mutex_delete(cond->waiter_lock);
    rt_free(cond);
}

int idcu_cond_wait(idcu_cond_t* cond, idcu_mutex_t* mutex)
{
    if (!cond || !mutex) {
        return -1;
    }
    rt_mutex_take(cond->waiter_lock, RT_WAITING_FOREVER);
    cond->waiters++;
    rt_mutex_release(cond->waiter_lock);
    
    rt_mutex_release(mutex->mutex);
    rt_sem_take(cond->sem, RT_WAITING_FOREVER);
    rt_mutex_take(mutex->mutex, RT_WAITING_FOREVER);
    
    return 0;
}

int idcu_cond_timedwait(idcu_cond_t* cond, idcu_mutex_t* mutex, uint32_t timeout_ms)
{
    if (!cond || !mutex) {
        return -1;
    }
    rt_mutex_take(cond->waiter_lock, RT_WAITING_FOREVER);
    cond->waiters++;
    rt_mutex_release(cond->waiter_lock);
    
    rt_mutex_release(mutex->mutex);
    rt_err_t result = rt_sem_take(cond->sem, timeout_ms);
    rt_mutex_take(mutex->mutex, RT_WAITING_FOREVER);
    
    if (result != RT_EOK) {
        rt_mutex_take(cond->waiter_lock, RT_WAITING_FOREVER);
        cond->waiters--;
        rt_mutex_release(cond->waiter_lock);
        return -1;
    }
    return 0;
}

int idcu_cond_signal(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    rt_mutex_take(cond->waiter_lock, RT_WAITING_FOREVER);
    if (cond->waiters > 0) {
        cond->waiters--;
        rt_sem_release(cond->sem);
    }
    rt_mutex_release(cond->waiter_lock);
    return 0;
}

int idcu_cond_broadcast(idcu_cond_t* cond)
{
    if (!cond) {
        return -1;
    }
    rt_mutex_take(cond->waiter_lock, RT_WAITING_FOREVER);
    while (cond->waiters > 0) {
        cond->waiters--;
        rt_sem_release(cond->sem);
    }
    rt_mutex_release(cond->waiter_lock);
    return 0;
}

idcu_rwlock_t* idcu_rwlock_create(void)
{
    idcu_rwlock_t* rwlock = (idcu_rwlock_t*)rt_malloc(sizeof(idcu_rwlock_t));
    if (!rwlock) {
        return NULL;
    }
    rwlock->mutex = rt_mutex_create("idcu_rwlock", RT_IPC_FLAG_PRIO);
    rwlock->read_sem = rt_sem_create("idcu_rwlock_r", 1, RT_IPC_FLAG_PRIO);
    rwlock->write_sem = rt_sem_create("idcu_rwlock_w", 1, RT_IPC_FLAG_PRIO);
    rwlock->readers = 0;
    rwlock->writers = 0;
    rwlock->write_waiters = 0;
    
    if (!rwlock->mutex || !rwlock->read_sem || !rwlock->write_sem) {
        if (rwlock->mutex) rt_mutex_delete(rwlock->mutex);
        if (rwlock->read_sem) rt_sem_delete(rwlock->read_sem);
        if (rwlock->write_sem) rt_sem_delete(rwlock->write_sem);
        rt_free(rwlock);
        return NULL;
    }
    return rwlock;
}

void idcu_rwlock_destroy(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return;
    }
    if (rwlock->mutex) rt_mutex_delete(rwlock->mutex);
    if (rwlock->read_sem) rt_sem_delete(rwlock->read_sem);
    if (rwlock->write_sem) rt_sem_delete(rwlock->write_sem);
    rt_free(rwlock);
}

int idcu_rwlock_rdlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    rt_mutex_take(rwlock->mutex, RT_WAITING_FOREVER);
    while (rwlock->writers > 0 || rwlock->write_waiters > 0) {
        rt_mutex_release(rwlock->mutex);
        rt_sem_take(rwlock->read_sem, RT_WAITING_FOREVER);
        rt_mutex_take(rwlock->mutex, RT_WAITING_FOREVER);
    }
    rwlock->readers++;
    rt_mutex_release(rwlock->mutex);
    return 0;
}

int idcu_rwlock_wrlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    rt_mutex_take(rwlock->mutex, RT_WAITING_FOREVER);
    rwlock->write_waiters++;
    while (rwlock->readers > 0 || rwlock->writers > 0) {
        rt_mutex_release(rwlock->mutex);
        rt_sem_take(rwlock->write_sem, RT_WAITING_FOREVER);
        rt_mutex_take(rwlock->mutex, RT_WAITING_FOREVER);
    }
    rwlock->write_waiters--;
    rwlock->writers++;
    rt_mutex_release(rwlock->mutex);
    return 0;
}

int idcu_rwlock_unlock(idcu_rwlock_t* rwlock)
{
    if (!rwlock) {
        return -1;
    }
    rt_mutex_take(rwlock->mutex, RT_WAITING_FOREVER);
    if (rwlock->readers > 0) {
        rwlock->readers--;
        if (rwlock->readers == 0 && rwlock->write_waiters > 0) {
            rt_sem_release(rwlock->write_sem);
        }
    } else if (rwlock->writers > 0) {
        rwlock->writers--;
        if (rwlock->write_waiters > 0) {
            rt_sem_release(rwlock->write_sem);
        } else {
            while (rwlock->readers > 0) {
                rt_sem_release(rwlock->read_sem);
            }
        }
    }
    rt_mutex_release(rwlock->mutex);
    return 0;
}
