#ifndef LOCK_H
#define LOCK_H

#include "error_code.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mutex;
#endif
    int initialized;
} Mutex;

typedef struct {
#ifdef _WIN32
    SRWLOCK rwlock;
#else
    pthread_rwlock_t rwlock;
#endif
    int initialized;
} RwLock;

int mutex_init(Mutex* lock);
void mutex_destroy(Mutex* lock);
int mutex_lock(Mutex* lock);
int mutex_unlock(Mutex* lock);

int rwlock_init(RwLock* lock);
void rwlock_destroy(RwLock* lock);
int rwlock_rdlock(RwLock* lock);
int rwlock_wrlock(RwLock* lock);
int rwlock_unlock(RwLock* lock);

#ifdef __cplusplus
}
#endif

#endif
