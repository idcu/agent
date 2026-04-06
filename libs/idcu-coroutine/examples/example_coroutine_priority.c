#include <idcu/coroutine/coroutine.h>
#include <stdio.h>

static int g_execution_order[10];
static int g_order_index = 0;

static idcu_CoroState coro_low_prio(idcu_Coroutine* coro) {
    printf("Low priority coroutine (prio=1) executed\n");
    g_execution_order[g_order_index++] = 1;
    return IDCU_CORO_FINISHED;
}

static idcu_CoroState coro_medium_prio(idcu_Coroutine* coro) {
    printf("Medium priority coroutine (prio=50) executed\n");
    g_execution_order[g_order_index++] = 50;
    return IDCU_CORO_FINISHED;
}

static idcu_CoroState coro_high_prio(idcu_Coroutine* coro) {
    printf("High priority coroutine (prio=100) executed\n");
    g_execution_order[g_order_index++] = 100;
    return IDCU_CORO_FINISHED;
}

int main(void) {
    printf("=== Coroutine Priority Example ===\n");
    printf("Higher priority coroutines should execute first\n\n");
    
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    printf("Creating coroutines in order: low -> medium -> high\n");
    
    int coro_low = idcu_coro_create(&sched, coro_low_prio, 1, 10, NULL);
    int coro_medium = idcu_coro_create(&sched, coro_medium_prio, 50, 10, NULL);
    int coro_high = idcu_coro_create(&sched, coro_high_prio, 100, 10, NULL);
    
    printf("\nReady count: %u\n", idcu_coro_get_ready_count(&sched));
    
    printf("\nRunning scheduler...\n\n");
    
    idcu_CoroState state;
    do {
        state = idcu_coro_sched_run(&sched);
    } while (state != IDCU_CORO_IDLE);
    
    printf("\nExecution order (should be 100, 50, 1): ");
    for (int i = 0; i < g_order_index; i++) {
        printf("%d ", g_execution_order[i]);
    }
    printf("\n");
    
    int correct = 1;
    if (g_order_index >= 3) {
        correct = (g_execution_order[0] == 100 && 
                   g_execution_order[1] == 50 && 
                   g_execution_order[2] == 1);
    }
    
    if (correct) {
        printf("Priority scheduling works correctly!\n");
    } else {
        printf("Warning: Priority order not as expected\n");
    }
    
    idcu_coro_destroy(&sched, coro_low);
    idcu_coro_destroy(&sched, coro_medium);
    idcu_coro_destroy(&sched, coro_high);
    
    idcu_coro_sched_destroy(&sched);
    
    printf("\n=== Example Complete ===\n");
    return 0;
}
