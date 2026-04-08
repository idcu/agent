#include <idcu/coroutine/coroutine.h>
#include <stdio.h>

static int g_counter = 0;

static idcu_CoroState coro_func(idcu_Coroutine *coro) {
    printf("Coroutine %d is running, user data: %p\n", coro->id, coro->user_data);
    g_counter++;
    return IDCU_CORO_FINISHED;
}

int main(void) {
    printf("=== Basic Coroutine Example ===\n");

    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);

    printf("Creating coroutines...\n");

    int coro1 = idcu_coro_create(&sched, coro_func, 10, 10, (void *)0x1234);
    int coro2 = idcu_coro_create(&sched, coro_func, 10, 10, (void *)0x5678);
    int coro3 = idcu_coro_create(&sched, coro_func, 10, 10, (void *)0x9abc);

    printf("Coroutine IDs: %d, %d, %d\n", coro1, coro2, coro3);
    printf("Ready count: %u\n", idcu_coro_get_ready_count(&sched));

    printf("\nRunning scheduler...\n");

    idcu_CoroState state;
    do {
        state = idcu_coro_sched_run(&sched);
    } while (state != IDCU_CORO_IDLE);

    printf("\nAll coroutines finished\n");
    printf("Counter: %d\n", g_counter);

    idcu_coro_destroy(&sched, coro1);
    idcu_coro_destroy(&sched, coro2);
    idcu_coro_destroy(&sched, coro3);

    idcu_coro_sched_destroy(&sched);

    printf("=== Example Complete ===\n");
    return 0;
}
