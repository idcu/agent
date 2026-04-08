#include "coroutine.h"
#include "idcu/common/error_code.h"
#include "idcu/metrics/metrics.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static uint64_t get_timestamp_us(void) {
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

static inline void circular_queue_init(idcu_CircularQueue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

static inline int circular_queue_enqueue(idcu_CircularQueue *q, uint32_t item) {
    if (q->count >= IDCU_CORO_MAX_COUNT)
        return -1;
    q->items[q->tail] = item;
    q->tail = (q->tail + 1) % IDCU_CORO_MAX_COUNT;
    q->count++;
    return 0;
}

static inline int circular_queue_dequeue(idcu_CircularQueue *q, uint32_t *item) {
    if (q->count == 0)
        return -1;
    *item = q->items[q->head];
    q->head = (q->head + 1) % IDCU_CORO_MAX_COUNT;
    q->count--;
    return 0;
}

static inline int circular_queue_remove(idcu_CircularQueue *q, uint32_t item) {
    uint32_t temp[IDCU_CORO_MAX_COUNT];
    uint32_t temp_count = 0;
    uint32_t current;
    int found = 0;

    while (q->count > 0) {
        if (circular_queue_dequeue(q, &current) != 0)
            break;
        if (current == item) {
            found = 1;
        } else {
            temp[temp_count++] = current;
        }
    }

    for (uint32_t i = 0; i < temp_count; i++) {
        circular_queue_enqueue(q, temp[i]);
    }

    return found ? 0 : -1;
}

static inline int find_coro_index(idcu_CoroScheduler *sched, uint32_t id) {
    if (id == 0 || id > IDCU_CORO_MAX_COUNT)
        return -1;
    uint32_t idx = sched->id_to_index[id];
    if (idx < sched->count && sched->coros[idx].id == id) {
        return (int)idx;
    }
    return -1;
}

static void add_to_ready_queue(idcu_CoroScheduler *sched, uint32_t coro_idx) {
    idcu_Coroutine *coro = &sched->coros[coro_idx];
    if (coro->prio >= IDCU_CORO_MAX_PRIO)
        return;
    circular_queue_enqueue(&sched->ready_queues[coro->prio], coro_idx);
}

static int remove_from_ready_queue(idcu_CoroScheduler *sched, uint32_t coro_idx) {
    idcu_Coroutine *coro = &sched->coros[coro_idx];
    if (coro->prio >= IDCU_CORO_MAX_PRIO)
        return -1;
    return circular_queue_remove(&sched->ready_queues[coro->prio], coro_idx);
}

static int get_next_ready_coro(idcu_CoroScheduler *sched) {
    for (int p = IDCU_CORO_MAX_PRIO - 1; p >= 0; p--) {
        uint32_t idx;
        if (circular_queue_dequeue(&sched->ready_queues[p], &idx) == 0) {
            return (int)idx;
        }
    }
    return -1;
}

void idcu_coro_sched_init(idcu_CoroScheduler *sched) {
    if (!sched)
        return;

    memset(sched, 0, sizeof(idcu_CoroScheduler));
    idcu_mutex_init(&sched->lock);

    for (uint32_t p = 0; p < IDCU_CORO_MAX_PRIO; p++) {
        circular_queue_init(&sched->ready_queues[p]);
    }

    sched->next_id = 1;
    sched->last_ts = get_timestamp_us();
}

void idcu_coro_sched_destroy(idcu_CoroScheduler *sched) {
    if (!sched)
        return;
    idcu_mutex_destroy(&sched->lock);
}

int idcu_coro_create(idcu_CoroScheduler *sched, idcu_CoroState (*func)(idcu_Coroutine *),
                     uint32_t prio, uint32_t timeslice, void *user_data) {
    if (!sched || !func)
        return IDCU_ERR_INVALID_PARAM;
    if (prio >= IDCU_CORO_MAX_PRIO)
        return IDCU_ERR_INVALID_PARAM;
    if (sched->count >= IDCU_CORO_MAX_COUNT)
        return IDCU_ERR_NO_MEMORY;

    int ret = idcu_mutex_lock(&sched->lock);
    if (ret != IDCU_ERR_SUCCESS)
        return ret;

    uint32_t idx = sched->count;
    idcu_Coroutine *coro = &sched->coros[idx];
    memset(coro, 0, sizeof(idcu_Coroutine));

    coro->id = sched->next_id++;
    coro->prio = prio;
    coro->state = IDCU_CORO_READY;
    coro->func = func;
    coro->timeslice = (timeslice == 0) ? IDCU_CORO_DEFAULT_TIMESLICE : timeslice;
    coro->user_data = user_data;

    sched->id_to_index[coro->id] = idx;
    add_to_ready_queue(sched, idx);
    sched->count++;

    /* TODO: Re-enable metrics when new API is fully integrated
    idcu_global_metrics_set("coro_total", sched->count);
    idcu_global_metrics_set("coro_ready", idcu_coro_get_ready_count(sched));
    */

    idcu_mutex_unlock(&sched->lock);
    return coro->id;
}

int idcu_coro_destroy(idcu_CoroScheduler *sched, uint32_t id) {
    if (!sched)
        return IDCU_ERR_INVALID_PARAM;

    int ret = idcu_mutex_lock(&sched->lock);
    if (ret != IDCU_ERR_SUCCESS)
        return ret;

    int idx = find_coro_index(sched, id);
    if (idx < 0) {
        idcu_mutex_unlock(&sched->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    idcu_Coroutine *coro = &sched->coros[idx];
    if (coro->state == IDCU_CORO_READY) {
        remove_from_ready_queue(sched, idx);
    }

    if ((uint32_t)idx < sched->count - 1) {
        sched->coros[idx] = sched->coros[sched->count - 1];
        sched->id_to_index[sched->coros[idx].id] = idx;
        if (sched->coros[idx].state == IDCU_CORO_READY) {
            remove_from_ready_queue(sched, sched->count - 1);
            add_to_ready_queue(sched, idx);
        }
    }

    sched->id_to_index[id] = 0;
    sched->count--;

    /* TODO: Re-enable metrics when new API is fully integrated
    idcu_global_metrics_set("coro_total", sched->count);
    idcu_global_metrics_set("coro_ready", idcu_coro_get_ready_count(sched));
    */

    idcu_mutex_unlock(&sched->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_coro_suspend(idcu_CoroScheduler *sched, uint32_t id) {
    if (!sched)
        return IDCU_ERR_INVALID_PARAM;

    int ret = idcu_mutex_lock(&sched->lock);
    if (ret != IDCU_ERR_SUCCESS)
        return ret;

    int idx = find_coro_index(sched, id);
    if (idx < 0) {
        idcu_mutex_unlock(&sched->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    idcu_Coroutine *coro = &sched->coros[idx];
    if (coro->state == IDCU_CORO_READY) {
        remove_from_ready_queue(sched, idx);
        coro->state = IDCU_CORO_SUSPENDED;
    } else if (coro->state == IDCU_CORO_RUNNING) {
        coro->state = IDCU_CORO_SUSPENDED;
    }

    /* TODO: Re-enable metrics when new API is fully integrated
    idcu_global_metrics_set("coro_ready", idcu_coro_get_ready_count(sched));
    */

    idcu_mutex_unlock(&sched->lock);
    return IDCU_ERR_SUCCESS;
}

int idcu_coro_resume(idcu_CoroScheduler *sched, uint32_t id) {
    if (!sched)
        return IDCU_ERR_INVALID_PARAM;

    int ret = idcu_mutex_lock(&sched->lock);
    if (ret != IDCU_ERR_SUCCESS)
        return ret;

    int idx = find_coro_index(sched, id);
    if (idx < 0) {
        idcu_mutex_unlock(&sched->lock);
        return IDCU_ERR_NOT_FOUND;
    }

    idcu_Coroutine *coro = &sched->coros[idx];
    if (coro->state == IDCU_CORO_SUSPENDED) {
        coro->state = IDCU_CORO_READY;
        add_to_ready_queue(sched, idx);
    }

    /* TODO: Re-enable metrics when new API is fully integrated
    idcu_global_metrics_set("coro_ready", idcu_coro_get_ready_count(sched));
    */

    idcu_mutex_unlock(&sched->lock);
    return IDCU_ERR_SUCCESS;
}

idcu_Coroutine *idcu_coro_get(idcu_CoroScheduler *sched, uint32_t id) {
    if (!sched)
        return NULL;

    int ret = idcu_mutex_lock(&sched->lock);
    if (ret != IDCU_ERR_SUCCESS)
        return NULL;

    int idx = find_coro_index(sched, id);
    idcu_Coroutine *result = NULL;
    if (idx >= 0) {
        result = &sched->coros[idx];
    }

    idcu_mutex_unlock(&sched->lock);
    return result;
}

uint32_t idcu_coro_get_ready_count(idcu_CoroScheduler *sched) {
    if (!sched)
        return 0;

    uint32_t total = 0;
    for (uint32_t p = 0; p < IDCU_CORO_MAX_PRIO; p++) {
        total += sched->ready_queues[p].count;
    }
    return total;
}

idcu_CoroState idcu_coro_sched_run(idcu_CoroScheduler *sched) {
    if (!sched)
        return IDCU_CORO_IDLE;

    int ret = idcu_mutex_lock(&sched->lock);
    if (ret != IDCU_ERR_SUCCESS)
        return IDCU_CORO_IDLE;

    int next_idx = get_next_ready_coro(sched);
    if (next_idx < 0) {
        idcu_mutex_unlock(&sched->lock);
        return IDCU_CORO_IDLE;
    }

    idcu_Coroutine *coro = &sched->coros[next_idx];
    sched->current = next_idx;
    sched->current_prio = coro->prio;
    coro->state = IDCU_CORO_RUNNING;

    /* TODO: Re-enable metrics when new API is fully integrated
    idcu_global_metrics_set("coro_running", 1);
    idcu_global_metrics_set("coro_ready", idcu_coro_get_ready_count(sched));
    */

    uint64_t start_ts = get_timestamp_us();
    coro->stats.last_switch_ts = start_ts;

    idcu_mutex_unlock(&sched->lock);

    idcu_CoroState result = coro->func(coro);

    ret = idcu_mutex_lock(&sched->lock);
    if (ret != IDCU_ERR_SUCCESS)
        return result;

    uint64_t end_ts = get_timestamp_us();
    uint64_t runtime = end_ts - start_ts;
    coro->stats.total_runtime_us += runtime;
    coro->stats.switch_count++;
    coro->stats.timeslice_used++;

    /* TODO: Re-enable metrics when new API is fully integrated
    idcu_global_metrics_inc("coro_switches", 1);
    idcu_global_metrics_inc("coro_runtime_us", runtime);
    idcu_global_metrics_set("coro_running", 0);
    */

    if (result == IDCU_CORO_RUNNING || result == IDCU_CORO_READY) {
        coro->state = IDCU_CORO_READY;
        if (coro->stats.timeslice_used < coro->timeslice) {
            add_to_ready_queue(sched, next_idx);
        } else {
            coro->stats.timeslice_used = 0;
            if (coro->prio > 0) {
                coro->prio--;
            }
            add_to_ready_queue(sched, next_idx);
        }
    } else if (result == IDCU_CORO_FINISHED) {
        coro->state = IDCU_CORO_FINISHED;
    } else {
        coro->state = result;
    }

    /* TODO: Re-enable metrics when new API is fully integrated
    idcu_global_metrics_set("coro_ready", idcu_coro_get_ready_count(sched));
    */

    idcu_mutex_unlock(&sched->lock);
    return result;
}
