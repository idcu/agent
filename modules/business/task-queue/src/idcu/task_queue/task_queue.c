#include <idcu/task_queue/task_queue.h>
#include <string.h>

int idcu_task_queue_module_init(idcu_TaskQueueModule* tqm) {
    if (!tqm) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(tqm, 0, sizeof(idcu_TaskQueueModule));
    tqm->initialized = 0;
    return IDCU_ERR_OK;
}

int idcu_task_queue_module_start(idcu_TaskQueueModule* tqm) {
    if (!tqm) {
        return IDCU_ERR_INVALID_ARG;
    }

    tqm->initialized = 1;
    return IDCU_ERR_OK;
}

int idcu_task_queue_module_stop(idcu_TaskQueueModule* tqm) {
    if (!tqm) {
        return IDCU_ERR_INVALID_ARG;
    }

    tqm->initialized = 0;
    return IDCU_ERR_OK;
}

void idcu_task_queue_module_destroy(idcu_TaskQueueModule* tqm) {
    if (!tqm) {
        return;
    }

    if (tqm->initialized) {
        idcu_task_queue_module_stop(tqm);
    }

    memset(tqm, 0, sizeof(idcu_TaskQueueModule));
}

uint64_t idcu_task_queue_module_add_task(idcu_TaskQueueModule* tqm, idcu_TaskFunc func, void* user_data) {
    if (!tqm || !tqm->initialized || !func) {
        return 0;
    }
    (void)user_data;
    return 1;
}

int idcu_task_queue_module_cancel_task(idcu_TaskQueueModule* tqm, uint64_t task_id) {
    if (!tqm || !tqm->initialized) {
        return IDCU_ERR_INVALID_STATE;
    }
    (void)task_id;
    return IDCU_ERR_OK;
}
