#ifndef IDCU_TASK_QUEUE_TASK_QUEUE_H
#define IDCU_TASK_QUEUE_TASK_QUEUE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*idcu_TaskFunc)(void* user_data);

typedef struct {
    idcu_TaskFunc func;
    void* user_data;
    uint64_t task_id;
} idcu_Task;

typedef struct {
    int initialized;
    void* user_data;
} idcu_TaskQueueModule;

int idcu_task_queue_module_init(idcu_TaskQueueModule* tqm);
int idcu_task_queue_module_start(idcu_TaskQueueModule* tqm);
int idcu_task_queue_module_stop(idcu_TaskQueueModule* tqm);
void idcu_task_queue_module_destroy(idcu_TaskQueueModule* tqm);

uint64_t idcu_task_queue_module_add_task(idcu_TaskQueueModule* tqm, idcu_TaskFunc func, void* user_data);
int idcu_task_queue_module_cancel_task(idcu_TaskQueueModule* tqm, uint64_t task_id);

#ifdef __cplusplus
}
#endif

#endif
