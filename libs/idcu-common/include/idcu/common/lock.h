#ifndef IDCU_COMMON_LOCK_H
#define IDCU_COMMON_LOCK_H

#include "error_code.h"

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION idcu_Mutex;
#else
#include <pthread.h>
typedef pthread_mutex_t idcu_Mutex;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_mutex_init(idcu_Mutex* mutex);
void idcu_mutex_destroy(idcu_Mutex* mutex);
int  idcu_mutex_lock(idcu_Mutex* mutex);
int  idcu_mutex_unlock(idcu_Mutex* mutex);
int  idcu_mutex_trylock(idcu_Mutex* mutex);

typedef struct
{
    idcu_Mutex* mutex;
} idcu_LockGuard;

int idcu_lock_guard_init(idcu_LockGuard* guard, idcu_Mutex* mutex);
void idcu_lock_guard_destroy(idcu_LockGuard* guard);

#ifdef __cplusplus
}
#endif

#endif
