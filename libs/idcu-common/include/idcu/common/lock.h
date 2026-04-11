#ifndef IDCU_COMMON_LOCK_H
#define IDCU_COMMON_LOCK_H

#include "error_code.h"
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
typedef struct {
    CRITICAL_SECTION cs;
#ifdef IDCU_DEBUG
    DWORD owner_thread;
    uint64_t lock_count;
    uint64_t total_locks;
#endif
} idcu_Mutex;
#else
#include <pthread.h>
typedef struct {
    pthread_mutex_t pmutex;
#ifdef IDCU_DEBUG
    pthread_t owner_thread;
    uint64_t lock_count;
    uint64_t total_locks;
#endif
} idcu_Mutex;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int  idcu_mutex_init(idcu_Mutex* mutex);
void idcu_mutex_destroy(idcu_Mutex* mutex);
int  idcu_mutex_lock(idcu_Mutex* mutex);
int  idcu_mutex_unlock(idcu_Mutex* mutex);
int  idcu_mutex_trylock(idcu_Mutex* mutex);

#ifdef IDCU_DEBUG
uint64_t idcu_mutex_get_lock_count(idcu_Mutex* mutex);
uint64_t idcu_mutex_get_total_locks(idcu_Mutex* mutex);
int idcu_mutex_is_held_by_current_thread(idcu_Mutex* mutex);
#endif

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
