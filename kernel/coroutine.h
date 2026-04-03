#ifndef COROUTINE_H
#define COROUTINE_H

#include "context.h"
#include "include/config.h"
#include "lock.h"

#define CORO_MAX_PRIO 64
#define CORO_MAX_COUNT 256
#define CORO_DEFAULT_TIMESLICE 10

typedef enum {
    CORO_IDLE,
    CORO_READY,
    CORO_RUNNING,
    CORO_SUSPENDED,
    CORO_FINISHED
} CoroState;

typedef struct Coroutine Coroutine;

typedef struct {
    uint64_t total_runtime_us;
    uint64_t switch_count;
    uint64_t last_switch_ts;
    uint32_t timeslice_used;
} CoroStats;

struct Coroutine {
    uint32_t id;
    uint32_t prio;
    CoroState state;
    StackContext ctx;
    CoroState (*func)(Coroutine*);
    uint32_t timeslice;
    CoroStats stats;
    void *user_data;
};

typedef struct {
    Coroutine coros[CORO_MAX_COUNT];
    uint32_t ready_queue[CORO_MAX_PRIO][CORO_MAX_COUNT];
    uint32_t ready_count[CORO_MAX_PRIO];
    uint32_t count;
    uint32_t current;
    uint32_t current_prio;
    uint32_t next_id;
    Mutex lock;
    uint64_t last_ts;
} CoroScheduler;

void coro_sched_init(CoroScheduler *sched);
void coro_sched_destroy(CoroScheduler *sched);
CoroState coro_sched_run(CoroScheduler *sched);
int coro_create(CoroScheduler *sched, CoroState (*func)(Coroutine*), uint32_t prio, uint32_t timeslice, void *user_data);
int coro_destroy(CoroScheduler *sched, uint32_t id);
int coro_suspend(CoroScheduler *sched, uint32_t id);
int coro_resume(CoroScheduler *sched, uint32_t id);
Coroutine* coro_get(CoroScheduler *sched, uint32_t id);
uint32_t coro_get_ready_count(CoroScheduler *sched);

#endif