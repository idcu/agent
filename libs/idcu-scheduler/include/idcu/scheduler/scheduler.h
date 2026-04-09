#ifndef IDCU_SCHEDULER_SCHEDULER_H
#define IDCU_SCHEDULER_SCHEDULER_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/scheduler/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_scheduler_init(idcu_Scheduler_Context** ctx);
void idcu_scheduler_destroy(idcu_Scheduler_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
