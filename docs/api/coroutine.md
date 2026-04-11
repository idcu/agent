# idcu-coroutine API 文档

用于协作式多任务的协程调度器。

## 协程调度器

```c
typedef struct idcu_CoroutineScheduler idcu_CoroutineScheduler;
```

### 调度器函数

```c
int idcu_coro_scheduler_init(idcu_CoroutineScheduler** out_scheduler);
void idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler);

int idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler);
void idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler);

int idcu_coro_scheduler_get_coroutine_count(idcu_CoroutineScheduler* scheduler);
```

## 协程

```c
typedef struct idcu_Coroutine idcu_Coroutine;
```

### 协程配置

```c
typedef struct {
    size_t stack_size;
    const char* name;
} idcu_CoroutineConfig;
```

### 协程函数

```c
int idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** out_coro,
                     void (*func)(void*), void* arg, const idcu_CoroutineConfig* config);
void idcu_coro_destroy(idcu_Coroutine* coro);

int idcu_coro_resume(idcu_Coroutine* coro);
int idcu_coro_yield(idcu_CoroutineScheduler* scheduler);

const char* idcu_coro_get_name(const idcu_Coroutine* coro);
idcu_CoroutineState idcu_coro_get_state(const idcu_Coroutine* coro);
```

## 协程状态

```c
typedef enum {
    IDCU_CORO_READY,
    IDCU_CORO_RUNNING,
    IDCU_CORO_SUSPENDED,
    IDCU_CORO_COMPLETED,
    IDCU_CORO_ERROR
} idcu_CoroutineState;
```

## 默认配置

```c
#define IDCU_COROUTINE_DEFAULT_STACK_SIZE (64 * 1024)
```

## 示例

```c
#include <idcu/coroutine/coroutine.h>
#include <stdio.h>

static void coroutine_func(void* arg) {
    int* counter = (int*)arg;
    printf("协程已启动，计数器: %d\n", *counter);
    
    for (int i = 0; i < 3; i++) {
        (*counter)++;
        printf("协程: %d\n", *counter);
        idcu_coro_y