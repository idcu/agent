#include "idcu/common/deadlock_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

static uintptr_t get_current_thread_id(void)
{
#ifdef _WIN32
    return (uintptr_t)GetCurrentThreadId();
#else
    return (uintptr_t)pthread_self();
#endif
}

static uint64_t get_current_time_ms(void)
{
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int idcu_deadlock_detector_init(idcu_DeadlockDetector* detector)
{
    if (!detector) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    memset(detector, 0, sizeof(idcu_DeadlockDetector));
    detector->enabled = 1;
    detector->deadlock_check_interval_ms = 1000;
    detector->last_deadlock_check_time = get_current_time_ms();
    
    return IDCU_ERR_OK;
}

void idcu_deadlock_detector_destroy(idcu_DeadlockDetector* detector)
{
    if (!detector) {
        return;
    }
    
    idcu_deadlock_detector_print_report(detector);
    memset(detector, 0, sizeof(idcu_DeadlockDetector));
}

void idcu_deadlock_detector_enable(idcu_DeadlockDetector* detector, int enable)
{
    if (!detector) {
        return;
    }
    detector->enabled = enable;
}

void idcu_deadlock_detector_set_check_interval(idcu_DeadlockDetector* detector, uint64_t interval_ms)
{
    if (!detector) {
        return;
    }
    detector->deadlock_check_interval_ms = interval_ms;
}

int idcu_deadlock_detector_register_lock(idcu_DeadlockDetector* detector, uintptr_t lock_id, const char* name)
{
    if (!detector) {
        return IDCU_ERR_INVALID_ARG;
    }
    
    if (!detector->enabled) {
        return IDCU_ERR_OK;
    }
    
    for (size_t i = 0; i < detector->num_locks; i++) {
        if (detector->lock_info[i].lock_id == lock_id) {
            return IDCU_ERR_OK;
        }
    }
    
    if (detector->num_locks >= IDCU_MAX_LOCKS) {
        return IDCU_ERR_OUT_OF_RANGE;
    }
    
    detector->lock_info[detector->num_locks].lock_id = lock_id;
    detector->lock_info[detector->num_locks].lock_name = name;
    detector->num_locks++;
    
    return IDCU_ERR_OK;
}

void idcu_deadlock_detector_unregister_lock(idcu_DeadlockDetector* detector, uintptr_t lock_id)
{
    if (!detector) {
        return;
    }
    
    if (!detector->enabled) {
        return;
    }
    
    for (size_t i = 0; i < detector->num_locks; i++) {
        if (detector->lock_info[i].lock_id == lock_id) {
            if (i < detector->num_locks - 1) {
                memmove(&detector->lock_info[i], &detector->lock_info[i + 1],
                       (detector->num_locks - i - 1) * sizeof(idcu_LockInfo));
            }
            detector->num_locks--;
            break;
        }
    }
}

static idcu_ThreadLockState* find_or_create_thread(idcu_DeadlockDetector* detector, uintptr_t thread_id)
{
    for (size_t i = 0; i < detector->num_threads; i++) {
        if (detector->threads[i].thread_id == thread_id) {
            return &detector->threads[i];
        }
    }
    
    if (detector->num_threads >= IDCU_MAX_THREADS) {
        return NULL;
    }
    
    idcu_ThreadLockState* thread = &detector->threads[detector->num_threads];
    memset(thread, 0, sizeof(idcu_ThreadLockState));
    thread->thread_id = thread_id;
    detector->num_threads++;
    
    return thread;
}

static void remove_wait_edge(idcu_DeadlockDetector* detector, uintptr_t thread_id)
{
    for (size_t i = 0; i < detector->num_edges; i++) {
        if (detector->wait_graph[i].from_thread == thread_id) {
            if (i < detector->num_edges - 1) {
                memmove(&detector->wait_graph[i], &detector->wait_graph[i + 1],
                       (detector->num_edges - i - 1) * sizeof(idcu_WaitGraphEdge));
            }
            detector->num_edges--;
            i--;
        }
    }
}

static int add_wait_edge(idcu_DeadlockDetector* detector, uintptr_t thread_id, uintptr_t lock_id)
{
    if (detector->num_edges >= IDCU_MAX_WAIT_GRAPH_EDGES) {
        return -1;
    }
    
    detector->wait_graph[detector->num_edges].from_thread = thread_id;
    detector->wait_graph[detector->num_edges].to_lock = lock_id;
    detector->num_edges++;
    
    return 0;
}

void idcu_deadlock_detector_on_lock_acquire(idcu_DeadlockDetector* detector, uintptr_t lock_id)
{
    if (!detector || !detector->enabled) {
        return;
    }
    
    uintptr_t thread_id = get_current_thread_id();
    idcu_ThreadLockState* thread = find_or_create_thread(detector, thread_id);
    if (!thread) {
        return;
    }
    
    if (thread->num_holding_locks < IDCU_MAX_LOCKS) {
        thread->holding_locks[thread->num_holding_locks++] = lock_id;
    }
    
    thread->waiting_for_lock = 0;
    remove_wait_edge(detector, thread_id);
}

void idcu_deadlock_detector_on_lock_release(idcu_DeadlockDetector* detector, uintptr_t lock_id)
{
    if (!detector || !detector->enabled) {
        return;
    }
    
    uintptr_t thread_id = get_current_thread_id();
    idcu_ThreadLockState* thread = find_or_create_thread(detector, thread_id);
    if (!thread) {
        return;
    }
    
    for (size_t i = 0; i < thread->num_holding_locks; i++) {
        if (thread->holding_locks[i] == lock_id) {
            if (i < thread->num_holding_locks - 1) {
                memmove(&thread->holding_locks[i], &thread->holding_locks[i + 1],
                       (thread->num_holding_locks - i - 1) * sizeof(uintptr_t));
            }
            thread->num_holding_locks--;
            break;
        }
    }
}

void idcu_deadlock_detector_on_lock_wait_start(idcu_DeadlockDetector* detector, uintptr_t lock_id)
{
    if (!detector || !detector->enabled) {
        return;
    }
    
    uintptr_t thread_id = get_current_thread_id();
    idcu_ThreadLockState* thread = find_or_create_thread(detector, thread_id);
    if (!thread) {
        return;
    }
    
    thread->waiting_for_lock = lock_id;
    thread->wait_start_time = get_current_time_ms();
    add_wait_edge(detector, thread_id, lock_id);
}

void idcu_deadlock_detector_on_lock_wait_end(idcu_DeadlockDetector* detector, uintptr_t lock_id)
{
    (void)lock_id;
    
    if (!detector || !detector->enabled) {
        return;
    }
    
    uintptr_t thread_id = get_current_thread_id();
    idcu_ThreadLockState* thread = find_or_create_thread(detector, thread_id);
    if (!thread) {
        return;
    }
    
    thread->waiting_for_lock = 0;
    remove_wait_edge(detector, thread_id);
}

static int dfs_visit(idcu_DeadlockDetector* detector, uintptr_t thread_id, 
                    uintptr_t* visited, size_t* visited_count,
                    uintptr_t* recursion_stack, size_t* recursion_stack_size)
{
    for (size_t i = 0; i < *recursion_stack_size; i++) {
        if (recursion_stack[i] == thread_id) {
            return 1;
        }
    }
    
    for (size_t i = 0; i < *visited_count; i++) {
        if (visited[i] == thread_id) {
            return 0;
        }
    }
    
    if (*visited_count >= IDCU_MAX_THREADS) {
        return 0;
    }
    visited[(*visited_count)++] = thread_id;
    
    if (*recursion_stack_size >= IDCU_MAX_THREADS) {
        return 0;
    }
    recursion_stack[(*recursion_stack_size)++] = thread_id;
    
    idcu_ThreadLockState* thread = NULL;
    for (size_t i = 0; i < detector->num_threads; i++) {
        if (detector->threads[i].thread_id == thread_id) {
            thread = &detector->threads[i];
            break;
        }
    }
    
    if (!thread || thread->waiting_for_lock == 0) {
        (*recursion_stack_size)--;
        return 0;
    }
    
    uintptr_t lock_id = thread->waiting_for_lock;
    
    for (size_t i = 0; i < detector->num_threads; i++) {
        idcu_ThreadLockState* other_thread = &detector->threads[i];
        for (size_t j = 0; j < other_thread->num_holding_locks; j++) {
            if (other_thread->holding_locks[j] == lock_id) {
                if (dfs_visit(detector, other_thread->thread_id, 
                           visited, visited_count,
                           recursion_stack, recursion_stack_size)) {
                    return 1;
                }
            }
        }
    }
    
    (*recursion_stack_size)--;
    return 0;
}

int idcu_deadlock_detector_check(idcu_DeadlockDetector* detector)
{
    if (!detector || !detector->enabled) {
        return 0;
    }
    
    uint64_t now = get_current_time_ms();
    if (now - detector->last_deadlock_check_time < detector->deadlock_check_interval_ms) {
        return 0;
    }
    detector->last_deadlock_check_time = now;
    
    uintptr_t visited[IDCU_MAX_THREADS];
    size_t visited_count = 0;
    uintptr_t recursion_stack[IDCU_MAX_THREADS];
    size_t recursion_stack_size = 0;
    
    for (size_t i = 0; i < detector->num_threads; i++) {
        memset(visited, 0, sizeof(visited));
        visited_count = 0;
        memset(recursion_stack, 0, sizeof(recursion_stack));
        recursion_stack_size = 0;
        
        if (dfs_visit(detector, detector->threads[i].thread_id,
                   visited, &visited_count,
                   recursion_stack, &recursion_stack_size)) {
            return 1;
        }
    }
    
    return 0;
}

static const char* get_lock_name(idcu_DeadlockDetector* detector, uintptr_t lock_id)
{
    for (size_t i = 0; i < detector->num_locks; i++) {
        if (detector->lock_info[i].lock_id == lock_id) {
            return detector->lock_info[i].lock_name ? detector->lock_info[i].lock_name : "unknown";
        }
    }
    return "unknown";
}

void idcu_deadlock_detector_print_report(idcu_DeadlockDetector* detector)
{
    if (!detector) {
        return;
    }
    
    printf("=== Deadlock Detector Report ===\n");
    printf("  Enabled: %s\n", detector->enabled ? "yes" : "no");
    printf("  Threads: %zu\n", detector->num_threads);
    printf("  Locks: %zu\n", detector->num_locks);
    printf("  Wait graph edges: %zu\n", detector->num_edges);
    printf("\n");
    
    for (size_t i = 0; i < detector->num_threads; i++) {
        idcu_ThreadLockState* thread = &detector->threads[i];
        printf("  Thread %p:\n", (void*)thread->thread_id);
        printf("    Holding locks: %zu\n", thread->num_holding_locks);
        for (size_t j = 0; j < thread->num_holding_locks; j++) {
            printf("      - %p (%s)\n", (void*)thread->holding_locks[j],
                   get_lock_name(detector, thread->holding_locks[j]));
        }
        if (thread->waiting_for_lock != 0) {
            printf("    Waiting for: %p (%s)\n", (void*)thread->waiting_for_lock,
                   get_lock_name(detector, thread->waiting_for_lock));
            uint64_t wait_time = get_current_time_ms() - thread->wait_start_time;
            printf("    Wait time: %llu ms\n", (unsigned long long)wait_time);
        }
        printf("\n");
    }
    
    if (idcu_deadlock_detector_check(detector)) {
        printf("  WARNING: Potential deadlock detected!\n");
    } else {
        printf("  No deadlock detected.\n");
    }
    
    printf("===============================\n");
}
