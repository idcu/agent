# idcu-scheduler

Task scheduler library for IDCU Agent.

## Features
- One-time tasks
- Periodic tasks
- Cron expressions
- Task priorities
- Task status monitoring
- Task cancellation
- Thread-safe implementation
- CMake build configuration

## Usage

```c
#include <idcu/scheduler/scheduler.h>

idcu_Scheduler_Context* ctx;
int ret = idcu_scheduler_init(&ctx);
if (ret == IDCU_ERR_OK) {
    // Start scheduler
    idcu_scheduler_start(ctx);
    
    // Add one-time task
    idcu_TaskId task1 = idcu_scheduler_add_once(ctx, "my_task", 1000, my_func, NULL);
    
    // Add periodic task
    idcu_TaskId task2 = idcu_scheduler_add_periodic(ctx, "periodic", 5000, my_func, NULL);
    
    // Add cron task
    idcu_TaskId task3 = idcu_scheduler_add_cron(ctx, "cron", "0 * * * *", my_func, NULL);
    
    // Get task status
    idcu_TaskStatus status;
    idcu_scheduler_get_task_status(ctx, task1, &status);
    
    // Cancel task
    idcu_scheduler_cancel_task(ctx, task2);
    
    // Stop scheduler
    idcu_scheduler_stop(ctx);
    
    // Destroy
    idcu_scheduler_destroy(ctx);
}
```

## API Reference
- `idcu_scheduler_init()` - Initialize scheduler
- `idcu_scheduler_start()` - Start scheduler
- `idcu_scheduler_stop()` - Stop scheduler
- `idcu_scheduler_destroy()` - Destroy scheduler
- `idcu_scheduler_add_once()` - Add one-time task
- `idcu_scheduler_add_periodic()` - Add periodic task
- `idcu_scheduler_add_cron()` - Add cron task
- `idcu_scheduler_cancel_task()` - Cancel task
- `idcu_scheduler_get_task_status()` - Get task status

## Building

```bash
cmake -B build && cmake --build build
```

## License
MIT
