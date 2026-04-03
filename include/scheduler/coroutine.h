#ifndef IDCU_SCHEDULER_COROUTINE_H
#define IDCU_SCHEDULER_COROUTINE_H

#include "scheduler/context.h"
#include "common/config.h"
#include "common/lock.h"

#define IDCU_CORO_MAX_PRIO 64
#define IDCU_CORO_MAX_COUNT 256
#define IDCU_CORO_DEFAULT_TIMESLICE 10

typedef enum {
    IDCU_CORO_IDLE,
    IDCU_CORO_READY,
    IDCU_CORO_RUNNING,
    IDCU_CORO_SUSPENDED,
    IDCU_CORO_FINISHED
} idcu_CoroState;

typedef struct idcu_Coroutine idcu_Coroutine;

typedef struct {
    uint64_t total_runtime_us;
    uint64_t switch_count;
    uint64_t last_switch_ts;
    uint32_t timeslice_used;
} idcu_CoroStats;

struct idcu_Coroutine {
    uint32_t id;
    uint32_t prio;
    idcu_CoroState state;
    idcu_StackContext ctx;
    idcu_CoroState (*func)(idcu_Coroutine*);
    uint32_t timeslice;
    idcu_CoroStats stats;
    void *user_data;
};

typedef struct {
    idcu_Coroutine coros[IDCU_CORO_MAX_COUNT];
    uint32_t ready_queue[IDCU_CORO_MAX_PRIO][IDCU_CORO_MAX_COUNT];
    uint32_t ready_count[IDCU_CORO_MAX_PRIO];
    uint32_t count;
    uint32_t current;
    uint32_t current_prio;
    uint32_t next_id;
    idcu_Mutex lock;
    uint64_t last_ts;
} idcu_CoroScheduler;

void idcu_coro_sched_init(idcu_CoroScheduler *sched);
void idcu_coro_sched_destroy(idcu_CoroScheduler *sched);
idcu_CoroState idcu_coro_sched_run(idcu_CoroScheduler *sched);
int idcu_coro_create(idcu_CoroScheduler *sched, idcu_CoroState (*func)(idcu_Coroutine*), uint32_t prio, uint32_t timeslice, void *user_data);
int idcu_coro_destroy(idcu_CoroScheduler *sched, uint32_t id);
int idcu_coro_suspend(idcu_CoroScheduler *sched, uint32_t id);
int idcu_coro_resume(idcu_CoroScheduler *sched, uint32_t id);
idcu_Coroutine* idcu_coro_get(idcu_CoroScheduler *sched, uint32_t id);
uint32_t idcu_coro_get_ready_count(idcu_CoroScheduler *sched);

#endif // IDCU_SCHEDULER_COROUTINE_H
