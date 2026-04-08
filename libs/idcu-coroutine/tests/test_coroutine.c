#include "idcu/coroutine/coroutine.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static int g_test_counter = 0;

static idcu_CoroState test_coro_func(idcu_Coroutine *coro) {
    g_test_counter++;
    return IDCU_CORO_FINISHED;
}

static void test_coro_sched_init_destroy(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    idcu_coro_sched_destroy(&sched);
    printf("test_coro_sched_init_destroy: PASS\n");
}

static void test_coro_create_destroy(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);

    int coro_id = idcu_coro_create(&sched, test_coro_func, 0, 10, NULL);
    if (coro_id == 0) {
        printf("test_coro_create_destroy: FAIL - idcu_coro_create should return non-zero ID\n");
        idcu_coro_sched_destroy(&sched);
        return;
    }

    idcu_Coroutine *coro = idcu_coro_get(&sched, coro_id);
    if (coro == NULL) {
        printf(
            "test_coro_create_destroy: FAIL - idcu_coro_get should find the created coroutine\n");
        idcu_coro_sched_destroy(&sched);
        return;
    }

    int ret = idcu_coro_destroy(&sched, coro_id);
    if (ret != 0) {
        printf("test_coro_create_destroy: FAIL - idcu_coro_destroy should succeed\n");
        idcu_coro_sched_destroy(&sched);
        return;
    }

    idcu_coro_sched_destroy(&sched);
    printf("test_coro_create_destroy: PASS\n");
}

static void test_coro_suspend_resume(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);

    int coro_id = idcu_coro_create(&sched, test_coro_func, 0, 10, NULL);
    if (coro_id == 0) {
        printf("test_coro_suspend_resume: FAIL - idcu_coro_create should succeed\n");
        idcu_coro_sched_destroy(&sched);
        return;
    }

    int ret = idcu_coro_suspend(&sched, coro_id);
    if (ret != 0) {
        printf("test_coro_suspend_resume: FAIL - idcu_coro_suspend should succeed\n");
        idcu_coro_sched_destroy(&sched);
        return;
    }

    ret = idcu_coro_resume(&sched, coro_id);
    if (ret != 0) {
        printf("test_coro_suspend_resume: FAIL - idcu_coro_resume should succeed\n");
        idcu_coro_sched_destroy(&sched);
        return;
    }

    idcu_coro_destroy(&sched, coro_id);
    idcu_coro_sched_destroy(&sched);
    printf("test_coro_suspend_resume: PASS\n");
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    printf("=== Coroutine Scheduler Tests ===\n");

    test_coro_sched_init_destroy();
    test_coro_create_destroy();
    test_coro_suspend_resume();

    printf("=== All Tests Completed ===\n");

    return 0;
}
