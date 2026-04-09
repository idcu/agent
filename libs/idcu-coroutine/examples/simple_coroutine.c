#include <idcu/coroutine/coroutine.h>
#include <stdio.h>

static void coroutine_a(void* arg) {
    (void)arg;
    for (int i = 0; i < 3; i++) {
        printf("Coroutine A: %d\n", i);
        idcu_coro_yield();
    }
}

static void coroutine_b(void* arg) {
    (void)arg;
    for (int i = 0; i < 5; i++) {
        printf("Coroutine B: %d\n", i);
        idcu_coro_yield();
    }
}

int main() {
    printf("=== Coroutine Scheduler Example ===\n\n");
    
    idcu_CoroutineScheduler* scheduler;
    int result = idcu_coro_scheduler_init(&scheduler);
    if (result != IDCU_OK) {
        fprintf(stderr, "Failed to init scheduler: %d\n", result);
        return 1;
    }
    
    idcu_Coroutine* coro_a;
    idcu_Coroutine* coro_b;
    idcu_CoroutineConfig config = {0};
    config.stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    config.name = "coro-a";
    
    result = idcu_coro_create(scheduler, &coro_a, coroutine_a, NULL, &config);
    if (result != IDCU_OK) {
        fprintf(stderr, "Failed to create coroutine A: %d\n", result);
        idcu_coro_scheduler_destroy(scheduler);
        return 1;
    }
    
    config.name = "coro-b";
    result = idcu_coro_create(scheduler, &coro_b, coroutine_b, NULL, &config);
    if (result != IDCU_OK) {
        fprintf(stderr, "Failed to create coroutine B: %d\n", result);
        idcu_coro_scheduler_destroy(scheduler);
        return 1;
    }
    
    printf("Starting scheduler...\n");
    result = idcu_coro_scheduler_run(scheduler);
    if (result != IDCU_OK) {
        fprintf(stderr, "Failed to run scheduler: %d\n", result);
        idcu_coro_scheduler_destroy(scheduler);
        return 1;
    }
    
    printf("\nScheduler finished.\n");
    
    result = idcu_coro_scheduler_destroy(scheduler);
    if (result != IDCU_OK) {
        fprintf(stderr, "Failed to destroy scheduler: %d\n", result);
        return 1;
    }
    
    printf("Done.\n");
    return 0;
}
