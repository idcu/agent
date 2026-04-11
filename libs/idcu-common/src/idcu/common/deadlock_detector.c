#include "idcu/common/deadlock_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
typedef DWORD idcu_ThreadId;
#define idcu_get_current_thread_id() GetCurrentThreadId()
#define idcu_thread_id_equal(a, b) ((a) == (b))
#else
#include <pthread.h>
typedef pthread_t idcu_ThreadId;
#define idcu_get_current_thread_id() pthread_self()
#define idcu_thread_id_equal(a, b) pthread_equal(a, b)
#endif

#define MAX_MUTEXES 1024
#define MAX_THREADS 1024
#define MAX_MUTEX_NAME_LEN 64

typedef struct {
    idcu_Mutex* mutex;
    char name[MAX_MUTEX_NAME_LEN];
    idcu_ThreadId owner;
    int is_locked;
} idcu_MutexInfo;

typedef struct {
    idcu_ThreadId thread_id;
    idcu_Mutex* waiting_for;
    int mutex_held_count;
    idcu_Mutex* mutexes_held[MAX_MUTEXES];
} idcu_ThreadInfo;

struct idcu_DeadlockDetector {
    idcu_MutexInfo mutexes[MAX_MUTEXES];
    int mutex_count;
    idcu_ThreadInfo threads[MAX_THREADS];
    int thread_count;
    idcu_Mutex detector_mutex;
};

static idcu_ThreadInfo* find_thread(idcu_DeadlockDetector* detector, idcu_ThreadId thread_id)
{
    for (int i = 0; i < detector->thread_count; i++) {
        if (idcu_thread_id_equal(detector->threads[i].thread_id, thread_id)) {
            return &detector->threads[i];
        }
    }
    return NULL;
}

static idcu_ThreadInfo* get_or_create_thread(idcu_DeadlockDetector* detector, idcu_ThreadId thread_id)
{
    idcu_ThreadInfo* thread = find_thread(detector, thread_id);
    if (thread) {
        return thread;
    }
    if (detector->thread_count >= MAX_THREADS) {
        return NULL;
    }
    thread = &detector->threads[detector->thread_count++];
    thread->thread_id = thread_id;
    thread->waiting_for = NULL;
    thread->mutex_held_count = 0;
    memset(thread->mutexes_held, 0, sizeof(thread->mutexes_held));
    return thread;
}

static idcu_MutexInfo* find_mutex(idcu_DeadlockDetector* detector, idcu_Mutex* mutex)
{
    for (int i = 0; i < detector->mutex_count; i++) {
        if (detector->mutexes[i].mutex == mutex) {
            return &detector->mutexes[i];
        }
    }
    return NULL;
}

int idcu_deadlock_detector_init(idcu_DeadlockDetector** detector)
{
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }
    *detector = (idcu_DeadlockDetector*)malloc(sizeof(idcu_DeadlockDetector));
    if (!*detector) {
        return IDCU_ERR_NO_MEMORY;
    }
    memset(*detector, 0, sizeof(idcu_DeadlockDetector));
    int ret = idcu_mutex_init(&(*detector)->detector_mutex);
    if (ret != IDCU_ERR_OK) {
        free(*detector);
        *detector = NULL;
        return ret;
    }
    return IDCU_ERR_OK;
}

void idcu_deadlock_detector_destroy(idcu_DeadlockDetector* detector)
{
    if (!detector) {
        return;
    }
    idcu_mutex_destroy(&detector->detector_mutex);
    free(detector);
}

int idcu_deadlock_detector_register_mutex(idcu_DeadlockDetector* detector, idcu_Mutex* mutex, const char* name)
{
    if (!detector || !mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }
    idcu_mutex_lock(&detector->detector_mutex);
    if (detector->mutex_count >= MAX_MUTEXES) {
        idcu_mutex_unlock(&detector->detector_mutex);
        return IDCU_ERR_LIMIT_EXCEEDED;
    }
    idcu_MutexInfo* info = &detector->mutexes[detector->mutex_count++];
    info->mutex = mutex;
    info->is_locked = 0;
    memset(&info->owner, 0, sizeof(info->owner));
    if (name) {
        strncpy(info->name, name, MAX_MUTEX_NAME_LEN - 1);
        info->name[MAX_MUTEX_NAME_LEN - 1] = '\0';
    } else {
        snprintf(info->name, MAX_MUTEX_NAME_LEN, "mutex_%d", detector->mutex_count - 1);
    }
    idcu_mutex_unlock(&detector->detector_mutex);
    return IDCU_ERR_OK;
}

void idcu_deadlock_detector_unregister_mutex(idcu_DeadlockDetector* detector, idcu_Mutex* mutex)
{
    if (!detector || !mutex) {
        return;
    }
    idcu_mutex_lock(&detector->detector_mutex);
    for (int i = 0; i < detector->mutex_count; i++) {
        if (detector->mutexes[i].mutex == mutex) {
            for (int j = i; j < detector->mutex_count - 1; j++) {
                detector->mutexes[j] = detector->mutexes[j + 1];
            }
            detector->mutex_count--;
            break;
        }
    }
    idcu_mutex_unlock(&detector->detector_mutex);
}

int idcu_deadlock_detector_on_lock_attempt(idcu_DeadlockDetector* detector, idcu_Mutex* mutex)
{
    if (!detector || !mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }
    idcu_ThreadId current_thread = idcu_get_current_thread_id();
    idcu_mutex_lock(&detector->detector_mutex);
    idcu_ThreadInfo* thread = get_or_create_thread(detector, current_thread);
    if (thread) {
        thread->waiting_for = mutex;
    }
    int result = idcu_deadlock_detector_check_deadlock(detector);
    idcu_mutex_unlock(&detector->detector_mutex);
    return result;
}

int idcu_deadlock_detector_on_lock_acquired(idcu_DeadlockDetector* detector, idcu_Mutex* mutex)
{
    if (!detector || !mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }
    idcu_ThreadId current_thread = idcu_get_current_thread_id();
    idcu_mutex_lock(&detector->detector_mutex);
    idcu_ThreadInfo* thread = get_or_create_thread(detector, current_thread);
    idcu_MutexInfo* mutex_info = find_mutex(detector, mutex);
    if (thread) {
        thread->waiting_for = NULL;
        if (thread->mutex_held_count < MAX_MUTEXES) {
            thread->mutexes_held[thread->mutex_held_count++] = mutex;
        }
    }
    if (mutex_info) {
        mutex_info->is_locked = 1;
        mutex_info->owner = current_thread;
    }
    idcu_mutex_unlock(&detector->detector_mutex);
    return IDCU_ERR_OK;
}

int idcu_deadlock_detector_on_unlock(idcu_DeadlockDetector* detector, idcu_Mutex* mutex)
{
    if (!detector || !mutex) {
        return IDCU_ERR_INVALID_PARAM;
    }
    idcu_ThreadId current_thread = idcu_get_current_thread_id();
    idcu_mutex_lock(&detector->detector_mutex);
    idcu_ThreadInfo* thread = find_thread(detector, current_thread);
    idcu_MutexInfo* mutex_info = find_mutex(detector, mutex);
    if (thread) {
        for (int i = 0; i < thread->mutex_held_count; i++) {
            if (thread->mutexes_held[i] == mutex) {
                for (int j = i; j < thread->mutex_held_count - 1; j++) {
                    thread->mutexes_held[j] = thread->mutexes_held[j + 1];
                }
                thread->mutex_held_count--;
                break;
            }
        }
    }
    if (mutex_info) {
        mutex_info->is_locked = 0;
        memset(&mutex_info->owner, 0, sizeof(mutex_info->owner));
    }
    idcu_mutex_unlock(&detector->detector_mutex);
    return IDCU_ERR_OK;
}

static int has_cycle(idcu_DeadlockDetector* detector, idcu_ThreadId start_thread, idcu_ThreadId current_thread, int* visited, int depth)
{
    if (depth > MAX_THREADS) {
        return 0;
    }
    for (int i = 0; i < detector->thread_count; i++) {
        if (idcu_thread_id_equal(detector->threads[i].thread_id, current_thread)) {
            if (visited[i]) {
                return idcu_thread_id_equal(current_thread, start_thread) ? 1 : 0;
            }
            visited[i] = 1;
            if (detector->threads[i].waiting_for) {
                idcu_MutexInfo* mutex_info = find_mutex(detector, detector->threads[i].waiting_for);
                if (mutex_info && mutex_info->is_locked) {
                    return has_cycle(detector, start_thread, mutex_info->owner, visited, depth + 1);
                }
            }
            return 0;
        }
    }
    return 0;
}

int idcu_deadlock_detector_check_deadlock(idcu_DeadlockDetector* detector)
{
    if (!detector) {
        return IDCU_ERR_INVALID_PARAM;
    }
    int visited[MAX_THREADS];
    for (int i = 0; i < detector->thread_count; i++) {
        memset(visited, 0, sizeof(visited));
        if (has_cycle(detector, detector->threads[i].thread_id, detector->threads[i].thread_id, visited, 0)) {
            return IDCU_ERR_DEADLOCK;
        }
    }
    return IDCU_ERR_OK;
}

void idcu_deadlock_detector_print_report(idcu_DeadlockDetector* detector)
{
    if (!detector) {
        return;
    }
    idcu_mutex_lock(&detector->detector_mutex);
    printf("=== Deadlock Detector Report ===\n");
    printf("Registered mutexes: %d\n", detector->mutex_count);
    for (int i = 0; i < detector->mutex_count; i++) {
        printf("  Mutex %s: %s\n", 
               detector->mutexes[i].name,
               detector->mutexes[i].is_locked ? "LOCKED" : "UNLOCKED");
    }
    printf("Active threads: %d\n", detector->thread_count);
    for (int i = 0; i < detector->thread_count; i++) {
        printf("  Thread %d: ", i);
        if (detector->threads[i].waiting_for) {
            idcu_MutexInfo* info = find_mutex(detector, detector->threads[i].waiting_for);
            printf("waiting for %s, ", info ? info->name : "unknown");
        }
        printf("holds %d mutexes\n", detector->threads[i].mutex_held_count);
    }
    printf("================================\n");
    idcu_mutex_unlock(&detector->detector_mutex);
}
