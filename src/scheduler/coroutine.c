#include "scheduler/coroutine.h"
#include "common/error_code.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static uint64_t get_timestamp_us(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (uint64_t)(count.QuadPart * 1000000 / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
#endif
}

static int find_coro_index(CoroScheduler *sched, uint32_t id)
{
    for (uint32_t i = 0; i < sched->count; i++) {
        if (sched->coros[i].id == id) {
            return i;
        }
    }
    return -1;
}

static void add_to_ready_queue(CoroScheduler *sched, uint32_t coro_idx)
{
    Coroutine *coro = &sched->coros[coro_idx];
    if (coro->prio >= CORO_MAX_PRIO) return;
    
    if (sched->ready_count[coro->prio] < CORO_MAX_COUNT) {
        sched->ready_queue[coro->prio][sched->ready_count[coro->prio]] = coro_idx;
        sched->ready_count[coro->prio]++;
    }
}

static int remove_from_ready_queue(CoroScheduler *sched, uint32_t coro_idx)
{
    Coroutine *coro = &sched->coros[coro_idx];
    if (coro->prio >= CORO_MAX_PRIO) return -1;
    
    for (uint32_t i = 0; i < sched->ready_count[coro->prio]; i++) {
        if (sched->ready_queue[coro->prio][i] == coro_idx) {
            for (uint32_t j = i; j < sched->ready_count[coro->prio] - 1; j++) {
                sched->ready_queue[coro->prio][j] = sched->ready_queue[coro->prio][j + 1];
            }
            sched->ready_count[coro->prio]--;
            return 0;
        }
    }
    return -1;
}

static int get_next_ready_coro(CoroScheduler *sched)
{
    for (int p = CORO_MAX_PRIO - 1; p >= 0; p--) {
        if (sched->ready_count[p] > 0) {
            uint32_t idx = sched->ready_queue[p][0];
            for (uint32_t i = 0; i < sched->ready_count[p] - 1; i++) {
                sched->ready_queue[p][i] = sched->ready_queue[p][i + 1];
            }
            sched->ready_count[p]--;
            return idx;
        }
    }
    return -1;
}

void coro_sched_init(CoroScheduler *sched)
{
    if (!sched) return;
    
    memset(sched, 0, sizeof(CoroScheduler));
    mutex_init(&sched->lock);
    sched->next_id = 1;
    sched->last_ts = get_timestamp_us();
}

void coro_sched_destroy(CoroScheduler *sched)
{
    if (!sched) return;
    mutex_destroy(&sched->lock);
}

int coro_create(CoroScheduler *sched, CoroState (*func)(Coroutine*), uint32_t prio, uint32_t timeslice, void *user_data)
{
    if (!sched || !func) return ERR_INVALID_PARAM;
    if (prio >= CORO_MAX_PRIO) return ERR_INVALID_PARAM;
    if (sched->count >= CORO_MAX_COUNT) return ERR_NO_MEMORY;
    
    int ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return ret;
    
    uint32_t idx = sched->count;
    Coroutine *coro = &sched->coros[idx];
    memset(coro, 0, sizeof(Coroutine));
    
    coro->id = sched->next_id++;
    coro->prio = prio;
    coro->state = CORO_READY;
    coro->func = func;
    coro->timeslice = (timeslice == 0) ? CORO_DEFAULT_TIMESLICE : timeslice;
    coro->user_data = user_data;
    
    add_to_ready_queue(sched, idx);
    sched->count++;
    
    mutex_unlock(&sched->lock);
    return coro->id;
}

int coro_destroy(CoroScheduler *sched, uint32_t id)
{
    if (!sched) return ERR_INVALID_PARAM;
    
    int ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return ret;
    
    int idx = find_coro_index(sched, id);
    if (idx < 0) {
        mutex_unlock(&sched->lock);
        return ERR_NOT_FOUND;
    }
    
    Coroutine *coro = &sched->coros[idx];
    if (coro->state == CORO_READY) {
        remove_from_ready_queue(sched, idx);
    }
    
    if ((uint32_t)idx < sched->count - 1) {
        sched->coros[idx] = sched->coros[sched->count - 1];
        if (sched->coros[idx].state == CORO_READY) {
            remove_from_ready_queue(sched, sched->count - 1);
            add_to_ready_queue(sched, idx);
        }
    }
    
    sched->count--;
    mutex_unlock(&sched->lock);
    return ERR_SUCCESS;
}

int coro_suspend(CoroScheduler *sched, uint32_t id)
{
    if (!sched) return ERR_INVALID_PARAM;
    
    int ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return ret;
    
    int idx = find_coro_index(sched, id);
    if (idx < 0) {
        mutex_unlock(&sched->lock);
        return ERR_NOT_FOUND;
    }
    
    Coroutine *coro = &sched->coros[idx];
    if (coro->state == CORO_READY) {
        remove_from_ready_queue(sched, idx);
        coro->state = CORO_SUSPENDED;
    } else if (coro->state == CORO_RUNNING) {
        coro->state = CORO_SUSPENDED;
    }
    
    mutex_unlock(&sched->lock);
    return ERR_SUCCESS;
}

int coro_resume(CoroScheduler *sched, uint32_t id)
{
    if (!sched) return ERR_INVALID_PARAM;
    
    int ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return ret;
    
    int idx = find_coro_index(sched, id);
    if (idx < 0) {
        mutex_unlock(&sched->lock);
        return ERR_NOT_FOUND;
    }
    
    Coroutine *coro = &sched->coros[idx];
    if (coro->state == CORO_SUSPENDED) {
        coro->state = CORO_READY;
        add_to_ready_queue(sched, idx);
    }
    
    mutex_unlock(&sched->lock);
    return ERR_SUCCESS;
}

Coroutine* coro_get(CoroScheduler *sched, uint32_t id)
{
    if (!sched) return NULL;
    
    int ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return NULL;
    
    int idx = find_coro_index(sched, id);
    Coroutine *result = NULL;
    if (idx >= 0) {
        result = &sched->coros[idx];
    }
    
    mutex_unlock(&sched->lock);
    return result;
}

uint32_t coro_get_ready_count(CoroScheduler *sched)
{
    if (!sched) return 0;
    
    int ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return 0;
    
    uint32_t total = 0;
    for (uint32_t p = 0; p < CORO_MAX_PRIO; p++) {
        total += sched->ready_count[p];
    }
    
    mutex_unlock(&sched->lock);
    return total;
}

CoroState coro_sched_run(CoroScheduler *sched)
{
    if (!sched) return CORO_IDLE;
    
    int ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return CORO_IDLE;
    
    int next_idx = get_next_ready_coro(sched);
    if (next_idx < 0) {
        mutex_unlock(&sched->lock);
        return CORO_IDLE;
    }
    
    Coroutine *coro = &sched->coros[next_idx];
    sched->current = next_idx;
    sched->current_prio = coro->prio;
    coro->state = CORO_RUNNING;
    
    uint64_t start_ts = get_timestamp_us();
    coro->stats.last_switch_ts = start_ts;
    
    mutex_unlock(&sched->lock);
    
    CoroState result = coro->func(coro);
    
    ret = mutex_lock(&sched->lock);
    if (ret != ERR_SUCCESS) return result;
    
    uint64_t end_ts = get_timestamp_us();
    uint64_t runtime = end_ts - start_ts;
    coro->stats.total_runtime_us += runtime;
    coro->stats.switch_count++;
    coro->stats.timeslice_used++;
    
    if (result == CORO_RUNNING || result == CORO_READY) {
        coro->state = CORO_READY;
        if (coro->stats.timeslice_used < coro->timeslice) {
            add_to_ready_queue(sched, next_idx);
        } else {
            coro->stats.timeslice_used = 0;
            if (coro->prio > 0) {
                coro->prio--;
            }
            add_to_ready_queue(sched, next_idx);
        }
    } else if (result == CORO_FINISHED) {
        coro->state = CORO_FINISHED;
    } else {
        coro->state = result;
    }
    
    mutex_unlock(&sched->lock);
    return result;
}