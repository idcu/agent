# idcu-task-queue

Task queue module for IDCU Agent, providing:
- Task scheduling
- Priority queue
- Task execution
- Task cancellation
- Task statistics

## Usage

```c
#include <idcu/task_queue/task_queue.h>

void my_task(void* user_data) {
    const char* msg = (const char*)user_data;
    printf("Task executed: %s\n", msg);
}

int main() {
    idcu_TaskQueueModule tqm;
    int ret = idcu_task_queue_module_init(&tqm);
    if (ret != IDCU_ERR_OK) {
        return 1;
    }
    
    ret = idcu_task_queue_module_start(&tqm);
    if (ret != IDCU_ERR_OK) {
        idcu_task_queue_module_destroy(&tqm);
        return 1;
    }
    
    uint64_t task_id = idcu_task_queue_module_add_task(&tqm, my_task, "Hello World");
    
    idcu_task_queue_module_cancel_task(&tqm, task_id);
    
    idcu_task_queue_module_stop(&tqm);
    idcu_task_queue_module_destroy(&tqm);
    
    return 0;
}
```
