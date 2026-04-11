#ifndef IDCU_SCHEDULER_SCHEDULER_H
#define IDCU_SCHEDULER_SCHEDULER_H

#include "idcu/common/error_code.h"
#include "idcu/scheduler/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_scheduler_init(idcu_Scheduler_Context** ctx);
int idcu_scheduler_start(idcu_Scheduler_Context* ctx);
void idcu_scheduler_stop(idcu_Scheduler_Context* ctx);
void idcu_scheduler_destroy(idcu_Scheduler_Context* ctx);

idcu_TaskId idcu_scheduler_add_once(idcu_Scheduler_Context* ctx, const char* name, uint64_t delay_ms, idcu_TaskFunc func, void* user_data);
idcu_TaskId idcu_scheduler_add_periodic(idcu_Scheduler_Context* ctx, const char* name, uint64_t interval_ms, idcu_TaskFunc func, void* user_data);
idcu_TaskId idcu_scheduler_add_cron(idcu_Scheduler_Context* ctx, const char* name, const char* cron_expr, idcu_TaskFunc func, void* user_data);

int idcu_scheduler_cancel_task(idcu_Scheduler_Context* ctx, idcu_TaskId id);
int idcu_scheduler_get_task_status(idcu_Scheduler_Context* ctx, idcu_TaskId id, idcu_TaskStatus* status);

#ifdef __cplusplus
}
#endif

#endif
