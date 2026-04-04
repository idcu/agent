# 教程四：协程调度详解

协程是 IDCU Agent 实现高效并发的关键技术。本教程将深入讲解协程调度器的原理和使用方法。

---

## 学习目标

完成本教程后，你将学会：
- 协程的基本概念和优势
- 如何创建和管理协程
- 协程状态转换
- 协程调度策略
- 协程的实际应用

---

## 一、协程基础

### 1.1 什么是协程？

协程（Coroutine）是一种轻量级的线程，也被称为"用户态线程"。与操作系统线程相比：

- **更轻量**：占用内存更少（通常几 KB）
- **切换更快**：用户态切换，无需内核介入
- **更可控**：调度由应用程序控制

### 1.2 协程状态

```c
#include "scheduler/coroutine.h"

// 协程的五种状态
typedef enum {
    IDCU_CORO_IDLE,       // 空闲 - 刚创建，尚未运行
    IDCU_CORO_READY,      // 就绪 - 等待调度
    IDCU_CORO_RUNNING,    // 运行中 - 正在执行
    IDCU_CORO_SUSPENDED,  // 挂起 - 主动让出 CPU
    IDCU_CORO_FINISHED    // 完成 - 执行结束
} idcu_CoroState;
```

---

## 二、协程调度器 API

### 2.1 初始化调度器

```c
#include "scheduler/coroutine.h"

idcu_CoroScheduler sched;

// 初始化调度器
idcu_coro_sched_init(&sched);

// 使用调度器...

// 销毁调度器
idcu_coro_sched_destroy(&sched);
```

### 2.2 创建协程

```c
// 协程函数原型
idcu_CoroState my_coroutine_func(idcu_Coroutine* coro)
{
    // 协程逻辑在这里
    // 返回新的状态
    
    // 示例：运行完就结束
    return IDCU_CORO_FINISHED;
}

// 创建协程
uint32_t coro_id = idcu_coro_create(
    &sched,           // 调度器指针
    my_coroutine_func, // 协程函数
    0,                 // 优先级（0-63，数字越小优先级越高）
    10,                // 时间片（单位：调度周期）
    NULL               // 用户数据
);

if (coro_id == 0) {
    idcu_log_error("Failed to create coroutine!");
} else {
    idcu_log_info("Created coroutine with ID: %u", coro_id);
}
```

### 2.3 运行调度器

```c
// 在主循环中运行调度器
while (running) {
    // 运行一次调度
    idcu_coro_sched_run(&sched);
    
    // 可以做其他事情...
}
```

---

## 三、协程函数详解

### 3.1 简单的协程示例

```c
#include "scheduler/coroutine.h"
#include "utils/log.h"

// 一个简单的计数器协程
idcu_CoroState counter_coroutine(idcu_Coroutine* coro)
{
    static int count = 0;
    
    idcu_log_info("[Coroutine %u] Count: %d", coro->id, ++count);
    
    // 挂起自己，让出 CPU
    return IDCU_CORO_SUSPENDED;
}

// 创建并使用
void example_usage(void)
{
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    // 创建协程
    uint32_t coro_id = idcu_coro_create(
        &sched, counter_coroutine, 0, 10, NULL
    );
    
    // 运行调度器几次
    for (int i = 0; i < 5; i++) {
        idcu_coro_sched_run(&sched);
    }
    
    idcu_coro_sched_destroy(&sched);
}
```

### 3.2 有状态的协程

使用协程的 `user_data` 字段存储状态：

```c
typedef struct {
    int count;
    int max_count;
} CoroData;

idcu_CoroState stateful_coroutine(idcu_Coroutine* coro)
{
    CoroData* data = (CoroData*)coro->user_data;
    
    if (!data) {
        return IDCU_CORO_FINISHED;
    }
    
    data->count++;
    idcu_log_info("[Coro %u] Count: %d/%d", 
                  coro->id, data->count, data->max_count);
    
    if (data->count >= data->max_count) {
        idcu_log_info("[Coro %u] Finished!", coro->id);
        return IDCU_CORO_FINISHED;
    }
    
    return IDCU_CORO_SUSPENDED;
}

void use_stateful_coro(void)
{
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    CoroData* data = (CoroData*)malloc(sizeof(CoroData));
    data->count = 0;
    data->max_count = 10;
    
    uint32_t coro_id = idcu_coro_create(
        &sched, stateful_coroutine, 0, 10, data
    );
    
    // 运行直到完成
    while (idcu_coro_get_ready_count(&sched) > 0) {
        idcu_coro_sched_run(&sched);
    }
    
    free(data);
    idcu_coro_sched_destroy(&sched);
}
```

---

## 四、协程控制

### 4.1 挂起和恢复

```c
void suspend_resume_example(void)
{
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    uint32_t coro_id = idcu_coro_create(
        &sched, my_coro_func, 0, 10, NULL
    );
    
    // 运行几次
    for (int i = 0; i < 3; i++) {
        idcu_coro_sched_run(&sched);
    }
    
    // 挂起协程
    idcu_coro_suspend(&sched, coro_id);
    idcu_log_info("Coroutine suspended");
    
    // 即使运行调度器，协程也不会执行
    for (int i = 0; i < 3; i++) {
        idcu_coro_sched_run(&sched);
    }
    
    // 恢复协程
    idcu_coro_resume(&sched, coro_id);
    idcu_log_info("Coroutine resumed");
    
    // 继续运行
    for (int i = 0; i < 3; i++) {
        idcu_coro_sched_run(&sched);
    }
    
    idcu_coro_sched_destroy(&sched);
}
```

### 4.2 销毁协程

```c
void destroy_example(void)
{
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    uint32_t coro_id = idcu_coro_create(
        &sched, my_coro_func, 0, 10, NULL
    );
    
    // 运行几次
    idcu_coro_sched_run(&sched);
    
    // 销毁协程
    int ret = idcu_coro_destroy(&sched, coro_id);
    if (ret == 0) {
        idcu_log_info("Coroutine destroyed");
    }
    
    idcu_coro_sched_destroy(&sched);
}
```

### 4.3 获取协程信息

```c
void get_coro_info(void)
{
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    uint32_t coro_id = idcu_coro_create(
        &sched, my_coro_func, 0, 10, NULL
    );
    
    // 获取协程指针
    idcu_Coroutine* coro = idcu_coro_get(&sched, coro_id);
    if (coro) {
        idcu_log_info("Coroutine ID: %u", coro->id);
        idcu_log_info("Priority: %u", coro->prio);
        idcu_log_info("State: %d", coro->state);
        idcu_log_info("Timeslice: %u", coro->timeslice);
        
        // 统计信息
        idcu_log_info("Total runtime: %lu us", coro->stats.total_runtime_us);
        idcu_log_info("Switch count: %lu", coro->stats.switch_count);
    }
    
    // 获取就绪队列中的协程数量
    uint32_t ready_count = idcu_coro_get_ready_count(&sched);
    idcu_log_info("Ready coroutines: %u", ready_count);
    
    idcu_coro_sched_destroy(&sched);
}
```

---

## 五、完整示例：多协程协作

让我们创建一个完整的示例，展示多个协程如何协作工作。

### 5.1 生产者-消费者模式

```c
#include "scheduler/coroutine.h"
#include "utils/log.h"
#include "common/lock.h"
#include <stdlib.h>

#define QUEUE_SIZE 10

// 共享队列
typedef struct {
    int items[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    idcu_Mutex lock;
} SharedQueue;

// 生产者数据
typedef struct {
    SharedQueue* queue;
    int produce_count;
    int produced;
} ProducerData;

// 消费者数据
typedef struct {
    SharedQueue* queue;
    int consume_count;
    int consumed;
} ConsumerData;

// 初始化队列
void queue_init(SharedQueue* q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    idcu_mutex_init(&q->lock);
}

// 入队
int queue_push(SharedQueue* q, int item)
{
    idcu_mutex_lock(&q->lock);
    if (q->count >= QUEUE_SIZE) {
        idcu_mutex_unlock(&q->lock);
        return -1;  // 队列满
    }
    q->items[q->tail] = item;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count++;
    idcu_mutex_unlock(&q->lock);
    return 0;
}

// 出队
int queue_pop(SharedQueue* q, int* item)
{
    idcu_mutex_lock(&q->lock);
    if (q->count == 0) {
        idcu_mutex_unlock(&q->lock);
        return -1;  // 队列空
    }
    *item = q->items[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    idcu_mutex_unlock(&q->lock);
    return 0;
}

// 生产者协程
idcu_CoroState producer_coroutine(idcu_Coroutine* coro)
{
    ProducerData* data = (ProducerData*)coro->user_data;
    
    if (data->produced >= data->produce_count) {
        idcu_log_info("[Producer %u] Done!", coro->id);
        return IDCU_CORO_FINISHED;
    }
    
    // 尝试生产
    int item = data->produced + 1;
    if (queue_push(data->queue, item) == 0) {
        data->produced++;
        idcu_log_info("[Producer %u] Produced: %d", coro->id, item);
    } else {
        idcu_log_debug("[Producer %u] Queue full, waiting...", coro->id);
    }
    
    return IDCU_CORO_SUSPENDED;
}

// 消费者协程
idcu_CoroState consumer_coroutine(idcu_Coroutine* coro)
{
    ConsumerData* data = (ConsumerData*)coro->user_data;
    
    if (data->consumed >= data->consume_count) {
        idcu_log_info("[Consumer %u] Done!", coro->id);
        return IDCU_CORO_FINISHED;
    }
    
    // 尝试消费
    int item;
    if (queue_pop(data->queue, &item) == 0) {
        data->consumed++;
        idcu_log_info("[Consumer %u] Consumed: %d", coro->id, item);
    } else {
        idcu_log_debug("[Consumer %u] Queue empty, waiting...", coro->id);
    }
    
    return IDCU_CORO_SUSPENDED;
}

void producer_consumer_example(void)
{
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    SharedQueue queue;
    queue_init(&queue);
    
    // 创建生产者数据
    ProducerData* prod_data = (ProducerData*)malloc(sizeof(ProducerData));
    prod_data->queue = &queue;
    prod_data->produce_count = 20;
    prod_data->produced = 0;
    
    // 创建消费者数据
    ConsumerData* cons_data = (ConsumerData*)malloc(sizeof(ConsumerData));
    cons_data->queue = &queue;
    cons_data->consume_count = 20;
    cons_data->consumed = 0;
    
    // 创建协程
    uint32_t prod_coro = idcu_coro_create(
        &sched, producer_coroutine, 1, 10, prod_data
    );
    uint32_t cons_coro = idcu_coro_create(
        &sched, consumer_coroutine, 1, 10, cons_data
    );
    
    idcu_log_info("Starting producer-consumer example...");
    
    // 运行调度器直到所有协程完成
    while (idcu_coro_get_ready_count(&sched) > 0) {
        idcu_coro_sched_run(&sched);
    }
    
    idcu_log_info("All coroutines finished!");
    
    // 清理
    free(prod_data);
    free(cons_data);
    idcu_mutex_destroy(&queue.lock);
    idcu_coro_sched_destroy(&sched);
}
```

---

## 六、最佳实践

### 6.1 协程设计原则

```c
// ✅ 好的做法：协程函数应该快速返回
idcu_CoroState good_coro(idcu_Coroutine* coro)
{
    // 做一点工作
    do_small_chunk_of_work();
    
    // 挂起，让其他协程也有机会运行
    return IDCU_CORO_SUSPENDED;
}

// ❌ 避免：协程函数长时间运行
idcu_CoroState bad_coro(idcu_Coroutine* coro)
{
    // 不要这样做！会阻塞其他协程
    for (int i = 0; i < 1000000; i++) {
        do_work();
    }
    return IDCU_CORO_FINISHED;
}
```

### 6.2 优先级设置

```c
// 实时任务 - 高优先级
uint32_t realtime_coro = idcu_coro_create(
    &sched, realtime_task, 0, 5, NULL
);

// 普通任务 - 中等优先级
uint32_t normal_coro = idcu_coro_create(
    &sched, normal_task, 32, 10, NULL
);

// 后台任务 - 低优先级
uint32_t background_coro = idcu_coro_create(
    &sched, background_task, 63, 20, NULL
);
```

### 6.3 资源管理

```c
typedef struct {
    FILE* fp;
    char* buffer;
} CoroResource;

idcu_CoroState resource_coro(idcu_Coroutine* coro)
{
    CoroResource* res = (CoroResource*)coro->user_data;
    
    // 使用资源...
    
    return IDCU_CORO_FINISHED;
}

void cleanup_coro(void)
{
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    CoroResource* res = (CoroResource*)malloc(sizeof(CoroResource));
    res->fp = fopen("data.txt", "r");
    res->buffer = (char*)malloc(1024);
    
    uint32_t coro_id = idcu_coro_create(
        &sched, resource_coro, 0, 10, res
    );
    
    // 运行...
    while (idcu_coro_get_ready_count(&sched) > 0) {
        idcu_coro_sched_run(&sched);
    }
    
    // ✅ 重要：手动清理资源
    if (res->fp) fclose(res->fp);
    if (res->buffer) free(res->buffer);
    free(res);
    
    idcu_coro_sched_destroy(&sched);
}
```

---

## 下一步

恭喜你完成了协程调度教程！接下来你可以：

- 阅读 [教程五：沙箱安全机制](./tutorial_05_sandbox.md)，学习如何保护系统安全
- 查看协程调度器的源代码，深入了解实现细节
- 尝试在你的模块中使用协程

---

祝你并发编程愉快！⚡
