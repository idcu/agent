#ifndef IDCU_COMMON_DEADLOCK_DETECTOR_H
#define IDCU_COMMON_DEADLOCK_DETECTOR_H

#include "idcu/common/error_code.h"
#include "idcu/common/lock.h"
#include <stdint.h>
#include <stddef.h>

#define IDCU_DEADLOCK_MAX_LOCKS 128
#define IDCU_DEADLOCK_MAX_THREADS 64
#define IDCU_DEADLOCK_CHECK_INTERVAL_MS 1000
#define IDCU_DEADLOCK_MAX_WAIT_MS 5000

#ifdef _WIN32
typedef DWORD idcu_ThreadId;
#else
typedef pthread_t idcu_ThreadId;
#endif

typedef struct {
    idcu_Mutex* lock;
    const char* lock_name;
    idcu_ThreadId owner_thread;
    uint64_t acquire_time;
    int in_use;
} idcu_LockInfo;

typedef struct {
    idcu_ThreadId thread_id;
    const char* thread_name;
    idcu_Mutex* waiting_for_lock;
    uint64_t wait_start_time;
    int is_waiting;
} idcu_ThreadInfo;

typedef void (*idcu_DeadlockCallback)(const char* thread1, const char* thread2, 
                                        const char* lock1, const char* lock2, void* user_data);

typedef struct {
    idcu_LockInfo locks[IDCU_DEADLOCK_MAX_LOCKS];
    idcu_ThreadInfo threads[IDCU_DEADLOCK_MAX_THREADS];
    idcu_Mutex detector_lock;
    volatile int enabled;
    volatile int should_exit;
    void* thread_handle;
    idcu_DeadlockCallback deadlock_callback;
    void* callback_data;
} idcu_DeadlockDetector;

int idcu_deadlock_detector_init(idcu_DeadlockDetector* detector);
void idcu_deadlock_detector_destroy(idcu_DeadlockDetector* detector);
int idcu_deadlock_detector_start(idcu_DeadlockDetector* detector);
int idcu_deadlock_detector_stop(idcu_DeadlockDetector* detector);
int idcu_deadlock_detector_register_lock(idcu_DeadlockDetector* detector, idcu_Mutex* lock, const char* name);
int idcu_deadlock_detector_unregister_lock(idcu_DeadlockDetector* detector, idcu_Mutex* lock);
int idcu_deadlock_detector_register_thread(idcu_DeadlockDetector* detector, const char* name);
int idcu_deadlock_detector_unregister_thread(idcu_DeadlockDetector* detector);
int idcu_deadlock_detector_before_lock(idcu_DeadlockDetector* detector, idcu_Mutex* lock);
int idcu_deadlock_detector_after_lock(idcu_DeadlockDetector* detector, idcu_Mutex* lock);
int idcu_deadlock_detector_before_unlock(idcu_DeadlockDetector* detector, idcu_Mutex* lock);
int idcu_deadlock_detector_set_callback(idcu_DeadlockDetector* detector, idcu_DeadlockCallback callback, void* user_data);
void idcu_deadlock_detector_check(idcu_DeadlockDetector* detector);

#endif // IDCU_COMMON_DEADLOCK_DETECTOR_H
