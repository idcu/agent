#include "coroutine.h"

void coro_sched_init(CoroScheduler *sched, Coroutine *coros, uint32_t n)
{
    sched->count = n;
    sched->current = 0;
    for (uint32_t i = 0; i < n; ++i) {
        sched->coros[i] = coros[i];
        sched->coros[i].state = CORO_READY;
    }
}

CoroState coro_sched_run(CoroScheduler *sched)
{
    if (sched->count == 0) return CORO_IDLE;

    Coroutine *c = &sched->coros[sched->current];
    if (c->state == CORO_READY) {
        c->state = CORO_RUNNING;
        c->state = c->func(c);
    }

    sched->current = (sched->current + 1) % sched->count;
    return c->state;
}