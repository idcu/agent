#include "test/test_framework.h"
#include "coroutine.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;
static int g_test_counter = 0;

static idcu_CoroState test_coro_func(idcu_Coroutine* coro) {
    g_test_counter++;
    return IDCU_CORO_FINISHED;
}

static void test_coro_sched_init_destroy(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

static void test_coro_create_destroy(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    int coro_id = idcu_coro_create(&sched, test_coro_func, 0, 10, NULL);
    IDCU_TEST_ASSERT(coro_id != 0, "idcu_coro_create should return non-zero ID");
    
    idcu_Coroutine* coro = idcu_coro_get(&sched, coro_id);
    IDCU_TEST_ASSERT(coro != NULL, "idcu_coro_get should find the created coroutine");
    
    int ret = idcu_coro_destroy(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_coro_destroy should succeed");
    
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

static void test_coro_suspend_resume(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    int coro_id = idcu_coro_create(&sched, test_coro_func, 0, 10, NULL);
    IDCU_TEST_ASSERT(coro_id != 0, "idcu_coro_create should succeed");
    
    int ret = idcu_coro_suspend(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_coro_suspend should succeed");
    
    ret = idcu_coro_resume(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_coro_resume should succeed");
    
    idcu_coro_destroy(&sched, coro_id);
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Coroutine Scheduler Tests");
    
    idcu_test_suite_add_test(&g_suite, "coro_sched_init_destroy", test_coro_sched_init_destroy);
    idcu_test_suite_add_test(&g_suite, "coro_create_destroy", test_coro_create_destroy);
    idcu_test_suite_add_test(&g_suite, "coro_suspend_resume", test_coro_suspend_resume);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
