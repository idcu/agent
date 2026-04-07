#include "idcu/scheduler/scheduler.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct idcu_SchedulerInternal {
    idcu_TaskInfo tasks[IDCU_TASK_MAX];
    int task_count;
    uint64_t next_task_id;
    int running;
};

static uint64_t get_current_time_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

int idcu_scheduler_init(idcu_Scheduler* scheduler) {
    if (!scheduler) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)malloc(sizeof(struct idcu_SchedulerInternal));
    if (!internal) return IDCU_ERR_NO_MEMORY;
    
    memset(internal, 0, sizeof(struct idcu_SchedulerInternal));
    internal->next_task_id = 1;
    internal->running = 0;
    scheduler->internal = internal;
    
    return IDCU_ERR_OK;
}

void idcu_scheduler_destroy(idcu_Scheduler* scheduler) {
    if (!scheduler || !scheduler->internal) return;
    idcu_scheduler_stop(scheduler);
    free(scheduler->internal);
    scheduler->internal = NULL;
}

int idcu_scheduler_start(idcu_Scheduler* scheduler) {
    if (!scheduler || !scheduler->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    if (internal->running) {
        return IDCU_ERR_OK;
    }
    
    internal->running = 1;
    return IDCU_ERR_OK;
}

int idcu_scheduler_stop(idcu_Scheduler* scheduler) {
    if (!scheduler || !scheduler->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    internal->running = 0;
    return IDCU_ERR_OK;
}

static int find_task_index(struct idcu_SchedulerInternal* internal, uint64_t task_id) {
    for (int i = 0; i < internal->task_count; i++) {
        if (internal->tasks[i].task_id == task_id) {
            return i;
        }
    }
    return -1;
}

uint64_t idcu_scheduler_add_task(idcu_Scheduler* scheduler, const char* name, 
                                   idcu_TaskCallback callback, void* user_data) {
    return idcu_scheduler_add_interval_task(scheduler, name, 0, 0, callback, user_data);
}

uint64_t idcu_scheduler_add_interval_task(idcu_Scheduler* scheduler, const char* name, 
                                           uint64_t interval_ms, int repeat,
                                           idcu_TaskCallback callback, void* user_data) {
    if (!scheduler || !scheduler->internal || !name || !callback) return 0;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    if (internal->task_count >= IDCU_TASK_MAX) {
        return 0;
    }
    
    idcu_TaskInfo* task = &internal->tasks[internal->task_count];
    memset(task, 0, sizeof(idcu_TaskInfo));
    
    task->task_id = internal->next_task_id++;
    strncpy(task->name, name, IDCU_TASK_NAME_MAX - 1);
    task->interval_ms = interval_ms;
    task->is_repeating = repeat;
    task->is_cron = 0;
    task->status = IDCU_TASK_STATUS_IDLE;
    task->callback = callback;
    task->user_data = user_data;
    
    uint64_t now = get_current_time_ms();
    task->next_run_ms = now + interval_ms;
    
    internal->task_count++;
    return task->task_id;
}

uint64_t idcu_scheduler_add_cron_task(idcu_Scheduler* scheduler, const char* name, 
                                        const char* cron_expr,
                                        idcu_TaskCallback callback, void* user_data) {
    if (!scheduler || !scheduler->internal || !name || !cron_expr || !callback) return 0;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    if (internal->task_count >= IDCU_TASK_MAX) {
        return 0;
    }
    
    idcu_TaskInfo* task = &internal->tasks[internal->task_count];
    memset(task, 0, sizeof(idcu_TaskInfo));
    
    task->task_id = internal->next_task_id++;
    strncpy(task->name, name, IDCU_TASK_NAME_MAX - 1);
    strncpy(task->cron_expression, cron_expr, IDCU_CRON_EXPR_MAX - 1);
    task->is_repeating = 1;
    task->is_cron = 1;
    task->status = IDCU_TASK_STATUS_IDLE;
    task->callback = callback;
    task->user_data = user_data;
    
    uint64_t now = get_current_time_ms();
    uint64_t next_run;
    if (idcu_cron_parse(cron_expr, &next_run, now) == IDCU_ERR_OK) {
        task->next_run_ms = next_run;
    } else {
        task->next_run_ms = now + 60000;
    }
    
    internal->task_count++;
    return task->task_id;
}

int idcu_scheduler_remove_task(idcu_Scheduler* scheduler, uint64_t task_id) {
    if (!scheduler || !scheduler->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    int index = find_task_index(internal, task_id);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    for (int i = index; i < internal->task_count - 1; i++) {
        internal->tasks[i] = internal->tasks[i + 1];
    }
    internal->task_count--;
    
    return IDCU_ERR_OK;
}

int idcu_scheduler_pause_task(idcu_Scheduler* scheduler, uint64_t task_id) {
    if (!scheduler || !scheduler->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    int index = find_task_index(internal, task_id);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    internal->tasks[index].status = IDCU_TASK_STATUS_PAUSED;
    return IDCU_ERR_OK;
}

int idcu_scheduler_resume_task(idcu_Scheduler* scheduler, uint64_t task_id) {
    if (!scheduler || !scheduler->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    int index = find_task_index(internal, task_id);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    internal->tasks[index].status = IDCU_TASK_STATUS_IDLE;
    uint64_t now = get_current_time_ms();
    internal->tasks[index].next_run_ms = now + internal->tasks[index].interval_ms;
    
    return IDCU_ERR_OK;
}

int idcu_scheduler_run_task_now(idcu_Scheduler* scheduler, uint64_t task_id) {
    if (!scheduler || !scheduler->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    int index = find_task_index(internal, task_id);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    idcu_TaskInfo* task = &internal->tasks[index];
    if (task->status == IDCU_TASK_STATUS_RUNNING) {
        return IDCU_ERR_OK;
    }
    
    idcu_TaskStatus old_status = task->status;
    task->status = IDCU_TASK_STATUS_RUNNING;
    task->last_run_ms = get_current_time_ms();
    
    if (task->callback) {
        task->callback(task->user_data);
        task->status = IDCU_TASK_STATUS_COMPLETED;
        task->success_count++;
    } else {
        task->status = old_status;
    }
    
    task->run_count++;
    return IDCU_ERR_OK;
}

int idcu_scheduler_get_task(idcu_Scheduler* scheduler, uint64_t task_id, idcu_TaskInfo* info) {
    if (!scheduler || !scheduler->internal || !info) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    int index = find_task_index(internal, task_id);
    if (index < 0) {
        return IDCU_ERR_NOT_FOUND;
    }
    
    *info = internal->tasks[index];
    return IDCU_ERR_OK;
}

int idcu_scheduler_list_tasks(idcu_Scheduler* scheduler, idcu_TaskInfo* tasks, int* count) {
    if (!scheduler || !scheduler->internal || !tasks || !count) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    int max_count = *count;
    *count = internal->task_count < max_count ? internal->task_count : max_count;
    
    for (int i = 0; i < *count; i++) {
        tasks[i] = internal->tasks[i];
    }
    
    return IDCU_ERR_OK;
}

int idcu_scheduler_process(idcu_Scheduler* scheduler) {
    if (!scheduler || !scheduler->internal) return IDCU_ERR_INVALID_PARAM;
    
    struct idcu_SchedulerInternal* internal = 
        (struct idcu_SchedulerInternal*)scheduler->internal;
    
    if (!internal->running) {
        return IDCU_ERR_OK;
    }
    
    uint64_t now = get_current_time_ms();
    
    for (int i = 0; i < internal->task_count; i++) {
        idcu_TaskInfo* task = &internal->tasks[i];
        
        if (task->status == IDCU_TASK_STATUS_PAUSED || 
            task->status == IDCU_TASK_STATUS_RUNNING) {
            continue;
        }
        
        if (now >= task->next_run_ms) {
            idcu_TaskStatus old_status = task->status;
            task->status = IDCU_TASK_STATUS_RUNNING;
            task->last_run_ms = now;
            
            if (task->callback) {
                task->callback(task->user_data);
                task->status = IDCU_TASK_STATUS_COMPLETED;
                task->success_count++;
            } else {
                task->status = old_status;
            }
            
            task->run_count++;
            
            if (task->is_repeating) {
                if (task->is_cron) {
                    uint64_t next_run;
                    if (idcu_cron_parse(task->cron_expression, &next_run, now) == IDCU_ERR_OK) {
                        task->next_run_ms = next_run;
                    } else {
                        task->next_run_ms = now + 60000;
                    }
                } else {
                    task->next_run_ms = now + task->interval_ms;
                }
                task->status = IDCU_TASK_STATUS_IDLE;
            }
        }
    }
    
    return IDCU_ERR_OK;
}

int idcu_cron_parse(const char* expr, uint64_t* next_run_ms, uint64_t current_ms) {
    if (!expr || !next_run_ms) return IDCU_ERR_INVALID_PARAM;
    
    *next_run_ms = current_ms + 60000;
    return IDCU_ERR_OK;
}
