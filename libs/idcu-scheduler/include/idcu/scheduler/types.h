#ifndef IDCU_SCHEDULER_TYPES_H
#define IDCU_SCHEDULER_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_SCHEDULER_MAX_ITEMS 1024

typedef uint64_t idcu_TaskId;
typedef enum {
    IDCU_TASK_TYPE_ONCE,
    IDCU_TASK_TYPE_PERIODIC,
    IDCU_TASK_TYPE_CRON
} idcu_TaskType;
typedef enum {
    IDCU_TASK_STATUS_PENDING,
    IDCU_TASK_STATUS_SCHEDULED,
    IDCU_TASK_STATUS_RUNNING,
    IDCU_TASK_STATUS_COMPLETED,
    IDCU_TASK_STATUS_FAILED,
    IDCU_TASK_STATUS_CANCELLED,
    IDCU_TASK_STATUS_PAUSED
} idcu_TaskStatus;
typedef enum {
    IDCU_TASK_PRIORITY_LOW,
    IDCU_TASK_PRIORITY_NORMAL,
    IDCU_TASK_PRIORITY_HIGH,
    IDCU_TASK_PRIORITY_CRITICAL
} idcu_TaskPriority;

typedef int (*idcu_TaskFunc)(void* user_data);
typedef void (*idcu_TaskCompleteCallback)(idcu_TaskId id, int result, void* user_data);

typedef struct idcu_Scheduler_Context {
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_Scheduler_Context;

#ifdef __cplusplus
}
#endif

#endif
