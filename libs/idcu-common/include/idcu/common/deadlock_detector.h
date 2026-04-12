#ifndef IDCU_COMMON_DEADLOCK_DETECTOR_H
#define IDCU_COMMON_DEADLOCK_DETECTOR_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MAX_LOCKS 256
#define IDCU_MAX_THREADS 64
#define IDCU_MAX_WAIT_GRAPH_EDGES 1024

typedef struct {
    uintptr_t lock_id;
    const char* lock_name;
} idcu_LockInfo;

typedef struct {
    uintptr_t thread_id;
    uintptr_t waiting_for_lock;
    uintptr_t holding_locks[IDCU_MAX_LOCKS];
    size_t num_holding_locks;
    uint64_t wait_start_time;
} idcu_ThreadLockState;

typedef struct {
    uintptr_t from_thread;
    uintptr_t to_lock;
} idcu_WaitGraphEdge;

typedef struct {
    idcu_ThreadLockState threads[IDCU_MAX_THREADS];
    size_t num_threads;
    idcu_WaitGraphEdge wait_graph[IDCU_MAX_WAIT_GRAPH_EDGES];
    size_t num_edges;
    idcu_LockInfo lock_info[IDCU_MAX_LOCKS];
    size_t num_locks;
    int enabled;
    uint64_t last_deadlock_check_time;
    uint64_t deadlock_check_interval_ms;
} idcu_DeadlockDetector;

int idcu_deadlock_detector_init(idcu_DeadlockDetector* detector);
void idcu_deadlock_detector_destroy(idcu_DeadlockDetector* detector);

void idcu_deadlock_detector_enable(idcu_DeadlockDetector* detector, int enable);
void idcu_deadlock_detector_set_check_interval(idcu_DeadlockDetector* detector, uint64_t interval_ms);

int idcu_deadlock_detector_register_lock(idcu_DeadlockDetector* detector, uintptr_t lock_id, const char* name);
void idcu_deadlock_detector_unregister_lock(idcu_DeadlockDetector* detector, uintptr_t lock_id);

void idcu_deadlock_detector_on_lock_acquire(idcu_DeadlockDetector* detector, uintptr_t lock_id);
void idcu_deadlock_detector_on_lock_release(idcu_DeadlockDetector* detector, uintptr_t lock_id);
void idcu_deadlock_detector_on_lock_wait_start(idcu_DeadlockDetector* detector, uintptr_t lock_id);
void idcu_deadlock_detector_on_lock_wait_end(idcu_DeadlockDetector* detector, uintptr_t lock_id);

int idcu_deadlock_detector_check(idcu_DeadlockDetector* detector);
void idcu_deadlock_detector_print_report(idcu_DeadlockDetector* detector);

#ifdef __cplusplus
}
#endif

#endif
