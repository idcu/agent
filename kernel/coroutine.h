#ifndef COROUTINE_H
#define COROUTINE_H

#include "context.h"

typedef enum {
    CORO_IDLE,
    CORO_READY,
    CORO_RUNNING,
    CORO_SUSPENDED,
    CORO_FINISHED
} CoroState;

typedef struct Coroutine Coroutine;

struct Coroutine {
    uint32_t id;
    uint32_t prio;
    CoroState state;
    StackContext ctx;
    CoroState (*func)(Coroutine*);
};

typedef struct {
    Coroutine coros[16];
    uint32_t count;
    uint32_t current;
} CoroScheduler;

void coro_sched_init(CoroScheduler *sched, Coroutine *coros, uint32_t n);
CoroState coro_sched_run(CoroScheduler *sched);

#endif