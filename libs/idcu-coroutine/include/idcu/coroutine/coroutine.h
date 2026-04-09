#ifndef IDCU_COROUTINE_COROUTINE_H
#define IDCU_COROUTINE_COROUTINE_H

#include <idcu/coroutine/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_coro_scheduler_init(idcu_CoroutineScheduler** scheduler);
int idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler);
int idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler);
int idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler);

int idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** coro,
                      idcu_CoroutineFunc func, void* arg,
                      const idcu_CoroutineConfig* config);
int idcu_coro_destroy(idcu_Coroutine* coro);
int idcu_coro_yield(void);
int idcu_coro_resume(idcu_Coroutine* coro);

idcu_CoroutineState idcu_coro_get_state(const idcu_Coroutine* coro);
const char* idcu_coro_get_name(const idcu_Coroutine* coro);
idcu_Coroutine* idcu_coro_current(void);

#ifdef __cplusplus
}
#endif

#endif
