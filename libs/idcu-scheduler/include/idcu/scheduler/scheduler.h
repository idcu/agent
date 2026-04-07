#ifndef IDCU_SCHEDULER_H
#define IDCU_SCHEDULER_H

#include "idcu/common/error_code.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_TASK_NAME_MAX 128
#define IDCU_TASK_MAX 64
#define IDCU_CRON_EXPR_MAX 128

typedef enum {
    IDCU_TASK_STATUS_IDLE = 0,
    IDCU_TASK_STATUS_RUNNING,
    IDCU_TASK_STATUS_COMPLETED,
    IDCU_TASK_STATUS_FAILED,
    IDCU_TASK_STATUS_PAUSED
} idcu_TaskStatus;

typedef void (*idcu_TaskCallback)(void* user_data);

typedef struct {
    uint64_t task_id;
    char name[IDCU_TASK_NAME_MAX];
    char cron_expression[IDCU_CRON_EXPR_MAX];
    uint64_t interval_ms;
    int is_repeating;
    int is_cron;
    idcu_TaskStatus status;
    uint64_t last_run_ms;
    uint64_t next_run_ms;
    uint64_t run_count;
    uint64_t success_count;
    uint64_t fail_count;
    idcu_TaskCallback callback;
    void* user_data;
} idcu_TaskInfo;

typedef struct {
    void* internal;
} idcu_Scheduler;

int idcu_scheduler_init(idcu_Scheduler* scheduler);
void idcu_scheduler_destroy(idcu_Scheduler* scheduler);

int idcu_scheduler_start(idcu_Scheduler* scheduler);
int idcu_scheduler_stop(idcu_Scheduler* scheduler);
int idcu_scheduler_process(idcu_Scheduler* scheduler);

uint64_t idcu_scheduler_add_task(idcu_Scheduler* scheduler, const char* name, 
                                   idcu_TaskCallback callback, void* user_data);
uint64_t idcu_scheduler_add_interval_task(idcu_Scheduler* scheduler, const char* name, 
                                           uint64_t interval_ms, int repeat,
                                           idcu_TaskCallback callback, void* user_data);
uint64_t idcu_scheduler_add_cron_task(idcu_Scheduler* scheduler, const char* name, 
                                        const char* cron_expr,
                                        idcu_TaskCallback callback, void* user_data);

int idcu_scheduler_remove_task(idcu_Scheduler* scheduler, uint64_t task_id);
int idcu_scheduler_pause_task(idcu_Scheduler* scheduler, uint64_t task_id);
int idcu_scheduler_resume_task(idcu_Scheduler* scheduler, uint64_t task_id);
int idcu_scheduler_run_task_now(idcu_Scheduler* scheduler, uint64_t task_id);

int idcu_scheduler_get_task(idcu_Scheduler* scheduler, uint64_t task_id, idcu_TaskInfo* info);
int idcu_scheduler_list_tasks(idcu_Scheduler* scheduler, idcu_TaskInfo* tasks, int* count);

int idcu_cron_parse(const char* expr, uint64_t* next_run_ms, uint64_t current_ms);

#ifdef __cplusplus
}
#endif

#endif
