#include "test_framework.h"
#include "coroutine.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

typedef struct {
    int counter;
    int expected_value;
    int done;
} CoroTestData;

static CoroTestData g_coro_data;

static idcu_CoroState test_coro_basic(idcu_Coroutine* coro) {
    (void)coro;
    g_coro_data.counter++;
    return IDCU_CORO_FINISHED;
}

static void test_coroutine_create_run(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    g_coro_data.counter = 0;
    
    int coro_id = idcu_coro_create(&sched, test_coro_basic, 4096, 10, NULL);
    IDCU_TEST_ASSERT(coro_id > 0, "coroutine create should succeed");
    
    int ret = idcu_coro_resume(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "coroutine resume should succeed");
    
    IDCU_TEST_ASSERT(g_coro_data.counter == 1, "counter should be 1");
    
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

static idcu_CoroState test_coro_yield(idcu_Coroutine* coro) {
    (void)coro;
    g_coro_data.counter++;
    if (g_coro_data.counter < 3) {
        return IDCU_CORO_YIELD;
    }
    return IDCU_CORO_FINISHED;
}

static void test_coroutine_yield(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    g_coro_data.counter = 0;
    
    int coro_id = idcu_coro_create(&sched, test_coro_yield, 4096, 10, NULL);
    IDCU_TEST_ASSERT(coro_id > 0, "coroutine create should succeed");
    
    int ret;
    ret = idcu_coro_resume(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "resume 1 should succeed");
    IDCU_TEST_ASSERT(g_coro_data.counter == 1, "counter should be 1");
    
    ret = idcu_coro_resume(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "resume 2 should succeed");
    IDCU_TEST_ASSERT(g_coro_data.counter == 2, "counter should be 2");
    
    ret = idcu_coro_resume(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "resume 3 should succeed");
    IDCU_TEST_ASSERT(g_coro_data.counter == 3, "counter should be 3");
    
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

static idcu_CoroState test_coro_priority_high(idcu_Coroutine* coro) {
    (void)coro;
    g_coro_data.counter = 100;
    return IDCU_CORO_FINISHED;
}

static idcu_CoroState test_coro_priority_low(idcu_Coroutine* coro) {
    (void)coro;
    g_coro_data.counter = 1;
    return IDCU_CORO_FINISHED;
}

static void test_coroutine_priority(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    g_coro_data.counter = 0;
    
    int low_coro_id = idcu_coro_create(&sched, test_coro_priority_low, 4096, 1, NULL);
    int high_coro_id = idcu_coro_create(&sched, test_coro_priority_high, 4096, 100, NULL);
    
    IDCU_TEST_ASSERT(low_coro_id > 0, "low priority coroutine create should succeed");
    IDCU_TEST_ASSERT(high_coro_id > 0, "high priority coroutine create should succeed");
    
    idcu_coro_resume(&sched, low_coro_id);
    IDCU_TEST_ASSERT(g_coro_data.counter == 1, "low priority should run first");
    
    idcu_coro_resume(&sched, high_coro_id);
    IDCU_TEST_ASSERT(g_coro_data.counter == 100, "high priority should run next");
    
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

static idcu_CoroState test_coro_with_data(idcu_Coroutine* coro) {
    CoroTestData* data = (CoroTestData*)idcu_coro_get_user_data(coro);
    data->counter++;
    return IDCU_CORO_FINISHED;
}

static void test_coroutine_user_data(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    CoroTestData data;
    data.counter = 0;
    
    int coro_id = idcu_coro_create(&sched, test_coro_with_data, 4096, 10, &data);
    IDCU_TEST_ASSERT(coro_id > 0, "coroutine create should succeed");
    
    idcu_coro_resume(&sched, coro_id);
    IDCU_TEST_ASSERT(data.counter == 1, "counter should be 1");
    
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

static void test_coroutine_destroy(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    int coro_id = idcu_coro_create(&sched, test_coro_basic, 4096, 10, NULL);
    IDCU_TEST_ASSERT(coro_id > 0, "coroutine create should succeed");
    
    int ret = idcu_coro_destroy(&sched, coro_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "coroutine destroy should succeed");
    
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

static void test_coroutine_multiple(void) {
    idcu_CoroScheduler sched;
    idcu_coro_sched_init(&sched);
    
    g_coro_data.counter = 0;
    
    int coro_ids[10];
    for (int i = 0; i < 10; i++) {
        coro_ids[i] = idcu_coro_create(&sched, test_coro_basic, 4096, 10, NULL);
        IDCU_TEST_ASSERT(coro_ids[i] > 0, "coroutine create should succeed");
    }
    
    for (int i = 0; i < 10; i++) {
        idcu_coro_resume(&sched, coro_ids[i]);
    }
    
    IDCU_TEST_ASSERT(g_coro_data.counter == 10, "counter should be 10");
    
    for (int i = 0; i < 10; i++) {
        idcu_coro_destroy(&sched, coro_ids[i]);
    }
    
    idcu_coro_sched_destroy(&sched);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Coroutine Scheduling Integration Tests");
    
    idcu_test_suite_add_test(&g_suite, "coroutine_create_run", test_coroutine_create_run);
    idcu_test_suite_add_test(&g_suite, "coroutine_yield", test_coroutine_yield);
    idcu_test_suite_add_test(&g_suite, "coroutine_priority", test_coroutine_priority);
    idcu_test_suite_add_test(&g_suite, "coroutine_user_data", test_coroutine_user_data);
    idcu_test_suite_add_test(&g_suite, "coroutine_destroy", test_coroutine_destroy);
    idcu_test_suite_add_test(&g_suite, "coroutine_multiple", test_coroutine_multiple);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
