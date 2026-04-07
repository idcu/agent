# IDCU Scheduler Module

定时任务调度模块，提供任务调度和管理功能。

## 功能特性

- 单次任务调度
- 周期性任务调度
- Cron 表达式任务调度
- 任务暂停/恢复
- 任务立即执行
- 任务状态监控

## 使用示例

```c
#include "idcu/scheduler/scheduler.h"

void my_task_callback(void* user_data) {
    printf("Task executed!\n");
}

idcu_Scheduler scheduler;
idcu_scheduler_init(&scheduler);
idcu_scheduler_start(&scheduler);

uint64_t task_id = idcu_scheduler_add_interval_task(
    &scheduler, "my-task", 5000, 1, my_task_callback, NULL);

while (1) {
    idcu_scheduler_process(&scheduler);
    usleep(100000);
}

idcu_scheduler_stop(&scheduler);
idcu_scheduler_destroy(&scheduler);
```

## 配置

暂无特殊配置。

## 依赖

- idcu-common
- idcu-log
