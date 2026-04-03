#ifndef IDCU_COMMON_LOCK_H
#define IDCU_COMMON_LOCK_H

#include "common/error_code.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif // IDCU_COMMON_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif // IDCU_COMMON_LOCK_H

typedef struct {
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mutex;
#endif // IDCU_COMMON_LOCK_H
    int initialized;
} idcu_Mutex;

typedef struct {
#ifdef _WIN32
    SRWLOCK rwlock;
#else
    pthread_rwlock_t rwlock;
#endif // IDCU_COMMON_LOCK_H
    int initialized;
} idcu_RwLock;

int idcu_mutex_init(idcu_Mutex* lock);
void idcu_mutex_destroy(idcu_Mutex* lock);
int idcu_mutex_lock(idcu_Mutex* lock);
int idcu_mutex_unlock(idcu_Mutex* lock);

int idcu_rwlock_init(idcu_RwLock* lock);
void idcu_rwlock_destroy(idcu_RwLock* lock);
int idcu_rwlock_rdlock(idcu_RwLock* lock);
int idcu_rwlock_wrlock(idcu_RwLock* lock);
int idcu_rwlock_unlock(idcu_RwLock* lock);

#ifdef __cplusplus
}
#endif // IDCU_COMMON_LOCK_H

#endif // IDCU_COMMON_LOCK_H
