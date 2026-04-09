#include <idcu/scheduler/scheduler.h>
#include <stdlib.h>
#include <string.h>

int idcu_scheduler_init(idcu_Scheduler_Context** ctx) {
    if (!ctx) {
        return IDCU_ERR_INVALID_ARG;
    }
    *ctx = NULL;
    return IDCU_ERR_OK;
}

void idcu_scheduler_destroy(idcu_Scheduler_Context* ctx) {
    (void)ctx;
}
