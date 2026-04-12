#include <idcu/testframework/testframework.h>
#include <idcu/coroutine/coroutine.h>
#include <stdio.h>

static int test_counter = 0;

static void test_coroutine_func(void* arg) {
    int* count = (int*)arg;
    for (int i = 0; i < *count; i++) {
        test_counter++;
        idcu_coro_yield();
    }
}

IDCU_TEST_CASE(coroutine, create) {
    idcu_CoroutineScheduler* scheduler = NULL;
    int result = idcu_coro_scheduler_init(&scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to init scheduler");
    
    idcu_Coroutine* coro = NULL;
    int count = 3;
    idcu_CoroutineConfig config = {0};
    config.stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    config.name = "test-coro";
    
    result = idcu_coro_create(scheduler, &coro, test_coroutine_func, &count, &config);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to create coroutine");
    IDCU_TEST_ASSERT_MSG(coro != NULL, "Coroutine is NULL");
    IDCU_TEST_ASSERT_MSG(idcu_coro_get_state(coro) == IDCU_CORO_READY, "Coroutine not in READY state");
    
    result = idcu_coro_destroy(coro);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to destroy coroutine");
    
    result = idcu_coro_scheduler_destroy(scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to destroy scheduler");
}

IDCU_TEST_CASE(coroutine, scheduler_run) {
    idcu_CoroutineScheduler* scheduler = NULL;
    int result = idcu_coro_scheduler_init(&scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to init scheduler");
    
    test_counter = 0;
    int count = 5;
    idcu_Coroutine* coro = NULL;
    idcu_CoroutineConfig config = {0};
    config.stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    
    result = idcu_coro_create(scheduler, &coro, test_coroutine_func, &count, &config);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to create coroutine");
    
    result = idcu_coro_scheduler_run(scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to run scheduler");
    
    IDCU_TEST_ASSERT_MSG(test_counter == 5, "Expected counter to be 5, got %d", test_counter);
    
    result = idcu_coro_scheduler_destroy(scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to destroy scheduler");
}

IDCU_TEST_CASE(coroutine, multiple) {
    idcu_CoroutineScheduler* scheduler = NULL;
    int result = idcu_coro_scheduler_init(&scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to init scheduler");
    
    test_counter = 0;
    int count1 = 3;
    int count2 = 4;
    idcu_Coroutine* coro1 = NULL;
    idcu_Coroutine* coro2 = NULL;
    idcu_CoroutineConfig config = {0};
    config.stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    
    result = idcu_coro_create(scheduler, &coro1, test_coroutine_func, &count1, &config);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to create coroutine 1");
    
    result = idcu_coro_create(scheduler, &coro2, test_coroutine_func, &count2, &config);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to create coroutine 2");
    
    result = idcu_coro_scheduler_run(scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to run scheduler");
    
    IDCU_TEST_ASSERT_MSG(test_counter == 7, "Expected counter to be 7, got %d", test_counter);
    
    result = idcu_coro_scheduler_destroy(scheduler);
    IDCU_TEST_ASSERT_MSG(result == IDCU_SUCCESS, "Failed to destroy scheduler");
}

int main(void) {
    return idcu_test_run_all();
}
