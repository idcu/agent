# idcu-task-queue

IDCU Agent 的任务队列模块，提供：
- 任务调度
- 优先级队列
- 任务执行
- 任务取消
- 任务统计

## 使用方法

```c
#include <idcu/task_queue/task_queue.h>

void my_task(void* user_data) {
    const char* msg = (const char*)user_data;
    printf("任务执行: %s\n", msg);
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

## 许可证

本库采用 [Apache License 2.0](../../../LICENSE) 许可证。
