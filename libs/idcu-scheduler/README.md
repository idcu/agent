# idcu-scheduler

IDCU Agent 的任务调度器库。

## 功能特性
- 一次性任务
- 周期性任务
- Cron 表达式
- 任务优先级
- 任务状态监控
- 任务取消
- 线程安全实现
- CMake 构建配置

## 使用方法

```c
#include <idcu/scheduler/scheduler.h>

idcu_Scheduler* ctx;
int ret = idcu_scheduler_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // 启动调度器
    idcu_scheduler_start(ctx);
    
    // 添加一次性任务
    idcu_TaskId task1 = idcu_scheduler_add_once(ctx, "my_task", 1000, my_func, NULL);
    
    // 添加周期性任务
    idcu_TaskId task2 = idcu_scheduler_add_periodic(ctx, "periodic", 5000, my_func, NULL);
    
    // 添加 Cron 任务
    idcu_TaskId task3 = idcu_scheduler_add_cron(ctx, "cron", "0 * * * *", my_func, NULL);
    
    // 获取任务状态
    idcu_TaskStatus status;
    idcu_scheduler_get_task_status(ctx, task1, &status);
    
    // 取消任务
    idcu_scheduler_cancel_task(ctx, task2);
    
    // 停止调度器
    idcu_scheduler_stop(ctx);
    
    // 销毁
    idcu_scheduler_destroy(ctx);
}
```

## API 参考
- `idcu_scheduler_init()` - 初始化调度器
- `idcu_scheduler_start()` - 启动调度器
- `idcu_scheduler_stop()` - 停止调度器
- `idcu_scheduler_destroy()` - 销毁调度器
- `idcu_scheduler_add_once()` - 添加一次性任务
- `idcu_scheduler_add_periodic()` - 添加周期性任务
- `idcu_scheduler_add_cron()` - 添加 Cron 任务
- `idcu_scheduler_cancel_task()` - 取消任务
- `idcu_scheduler_get_task_status()` - 获取任务状态

## 构建

```bash
cmake -B build && cmake --build build
```

## 许可证

本库采用 [Apache License 2.0](../../LICENSE) 许可证。
