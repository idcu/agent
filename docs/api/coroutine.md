# idcu-coroutine API Documentation

Coroutine scheduler for cooperative multitasking.

## Coroutine Scheduler

```c
typedef struct idcu_CoroutineScheduler idcu_CoroutineScheduler;
```

### Scheduler Functions

```c
int idcu_coro_scheduler_init(idcu_CoroutineScheduler** out_scheduler);
void idcu_coro_scheduler_destroy(idcu_CoroutineScheduler* scheduler);

int idcu_coro_scheduler_run(idcu_CoroutineScheduler* scheduler);
void idcu_coro_scheduler_stop(idcu_CoroutineScheduler* scheduler);

int idcu_coro_scheduler_get_coroutine_count(idcu_CoroutineScheduler* scheduler);
```

## Coroutine

```c
typedef struct idcu_Coroutine idcu_Coroutine;
```

### Coroutine Configuration

```c
typedef struct {
    size_t stack_size;
    const char* name;
} idcu_CoroutineConfig;
```

### Coroutine Functions

```c
int idcu_coro_create(idcu_CoroutineScheduler* scheduler, idcu_Coroutine** out_coro,
                     void (*func)(void*), void* arg, const idcu_CoroutineConfig* config);
void idcu_coro_destroy(idcu_Coroutine* coro);

int idcu_coro_resume(idcu_Coroutine* coro);
int idcu_coro_yield(idcu_CoroutineScheduler* scheduler);

const char* idcu_coro_get_name(const idcu_Coroutine* coro);
idcu_CoroutineState idcu_coro_get_state(const idcu_Coroutine* coro);
```

## Coroutine States

```c
typedef enum {
    IDCU_CORO_READY,
    IDCU_CORO_RUNNING,
    IDCU_CORO_SUSPENDED,
    IDCU_CORO_COMPLETED,
    IDCU_CORO_ERROR
} idcu_CoroutineState;
```

## Default Configuration

```c
#define IDCU_COROUTINE_DEFAULT_STACK_SIZE (64 * 1024)
```

## Example

```c
#include <idcu/coroutine/coroutine.h>
#include <stdio.h>

static void coroutine_func(void* arg) {
    int* counter = (int*)arg;
    printf("Coroutine started, counter: %d\n", *counter);
    
    for (int i = 0; i < 3; i++) {
        (*counter)++;
        printf("Coroutine: %d\n", *counter);
        idcu_coro_yield(idcu_coro_get_current_scheduler());
    }
    
    printf("Coroutine finished\n");
}

int main(void) {
    idcu_CoroutineScheduler* scheduler = NULL;
    idcu_coro_scheduler_init(&scheduler);
    
    int counter = 0;
    idcu_CoroutineConfig config = {
        .stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE,
        .name = "example-coro"
    };
    
    idcu_Coroutine* coro = NULL;
    idcu_coro_create(scheduler, &coro, coroutine_func, &counter, &config);
    
    // Run the scheduler
    while (idcu_coro_get_state(coro) != IDCU_CORO_COMPLETED) {
        idcu_coro_resume(coro);
    }
    
    printf("Final counter: %d\n", counter);
    
    idcu_coro_destroy(coro);
    idcu_coro_scheduler_destroy(scheduler);
    return 0;
}
```
