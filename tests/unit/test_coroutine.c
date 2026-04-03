#include "test/test_framework.h"
#include "scheduler/coroutine.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;
static int g_test_counter = 0;

static CoroState test_coro_func(Coroutine* coro) {
    g_test_counter++;
    return CORO_FINISHED;
}

static void test_coro_sched_init_destroy(void) {
    CoroScheduler sched;
    coro_sched_init(&sched);
    TEST_ASSERT(coro_get_count(&sched) == 0, "Initial count should be 0");
    coro_sched_destroy(&sched);
    TEST_PASS();
}

static void test_coro_create(void) {
    CoroScheduler sched;
    coro_sched_init(&sched);
    
    Coroutine* coro = coro_create(&sched, CORO_PRIO_NORMAL, test_coro_func, NULL);
    TEST_ASSERT(coro != NULL, "coro_create should return non-NULL");
    TEST_ASSERT(coro_get_count(&sched) == 1, "Count should be 1 after create");
    
    coro_destroy(&sched, coro);
    TEST_ASSERT(coro_get_count(&sched) == 0, "Count should be 0 after destroy");
    
    coro_sched_destroy(&sched);
    TEST_PASS();
}

static void test_coro_priority(void) {
    CoroScheduler sched;
    coro_sched_init(&sched);
    
    Coroutine* coro_low = coro_create(&sched, CORO_PRIO_LOW, test_coro_func, NULL);
    Coroutine* coro_high = coro_create(&sched, CORO_PRIO_HIGH, test_coro_func, NULL);
    
    TEST_ASSERT(coro_low->prio == CORO_PRIO_LOW, "Low priority coroutine should have CORO_PRIO_LOW");
    TEST_ASSERT(coro_high->prio == CORO_PRIO_HIGH, "High priority coroutine should have CORO_PRIO_HIGH");
    
    coro_destroy(&sched, coro_low);
    coro_destroy(&sched, coro_high);
    
    coro_sched_destroy(&sched);
    TEST_PASS();
}

static void test_coro_get_by_id(void) {
    CoroScheduler sched;
    coro_sched_init(&sched);
    
    Coroutine* coro = coro_create(&sched, CORO_PRIO_NORMAL, test_coro_func, NULL);
    uint32_t id = coro->id;
    
    Coroutine* found = coro_get_by_id(&sched, id);
    TEST_ASSERT(found == coro, "Should find coroutine by ID");
    
    Coroutine* not_found = coro_get_by_id(&sched, 9999);
    TEST_ASSERT(not_found == NULL, "Should return NULL for invalid ID");
    
    coro_destroy(&sched, coro);
    coro_sched_destroy(&sched);
    TEST_PASS();
}

static void test_coro_suspend_resume(void) {
    CoroScheduler sched;
    coro_sched_init(&sched);
    
    Coroutine* coro = coro_create(&sched, CORO_PRIO_NORMAL, test_coro_func, NULL);
    TEST_ASSERT(coro->state == CORO_READY, "New coroutine should be in READY state");
    
    int ret = coro_suspend(&sched, coro);
    TEST_ASSERT(ret == ERR_OK, "coro_suspend should succeed");
    TEST_ASSERT(coro->state == CORO_SUSPENDED, "Coroutine should be SUSPENDED after suspend");
    
    ret = coro_resume(&sched, coro);
    TEST_ASSERT(ret == ERR_OK, "coro_resume should succeed");
    TEST_ASSERT(coro->state == CORO_READY, "Coroutine should be READY after resume");
    
    coro_destroy(&sched, coro);
    coro_sched_destroy(&sched);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Coroutine Scheduler Tests");
    
    test_suite_add_test(&g_suite, "coro_sched_init_destroy", test_coro_sched_init_destroy);
    test_suite_add_test(&g_suite, "coro_create", test_coro_create);
    test_suite_add_test(&g_suite, "coro_priority", test_coro_priority);
    test_suite_add_test(&g_suite, "coro_get_by_id", test_coro_get_by_id);
    test_suite_add_test(&g_suite, "coro_suspend_resume", test_coro_suspend_resume);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
