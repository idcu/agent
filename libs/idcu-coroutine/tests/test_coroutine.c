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

TEST_CASE(coroutine_create) {
    idcu_CoroutineScheduler* scheduler = NULL;
    int result = idcu_coro_scheduler_init(&scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to init scheduler");
    
    idcu_Coroutine* coro = NULL;
    int count = 3;
    idcu_CoroutineConfig config = {0};
    config.stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    config.name = "test-coro";
    
    result = idcu_coro_create(scheduler, &coro, test_coroutine_func, &count, &config);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to create coroutine");
    TEST_ASSERT(coro != NULL, "Coroutine is NULL");
    TEST_ASSERT(idcu_coro_get_state(coro) == IDCU_CORO_READY, "Coroutine not in READY state");
    
    result = idcu_coro_destroy(coro);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to destroy coroutine");
    
    result = idcu_coro_scheduler_destroy(scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to destroy scheduler");
    
    TEST_PASS();
}

TEST_CASE(coroutine_scheduler_run) {
    idcu_CoroutineScheduler* scheduler = NULL;
    int result = idcu_coro_scheduler_init(&scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to init scheduler");
    
    test_counter = 0;
    int count = 5;
    idcu_Coroutine* coro = NULL;
    idcu_CoroutineConfig config = {0};
    config.stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    
    result = idcu_coro_create(scheduler, &coro, test_coroutine_func, &count, &config);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to create coroutine");
    
    result = idcu_coro_scheduler_run(scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to run scheduler");
    
    TEST_ASSERT(test_counter == 5, "Expected counter to be 5, got %d", test_counter);
    
    result = idcu_coro_scheduler_destroy(scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to destroy scheduler");
    
    TEST_PASS();
}

TEST_CASE(coroutine_multiple) {
    idcu_CoroutineScheduler* scheduler = NULL;
    int result = idcu_coro_scheduler_init(&scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to init scheduler");
    
    test_counter = 0;
    int count1 = 3;
    int count2 = 4;
    idcu_Coroutine* coro1 = NULL;
    idcu_Coroutine* coro2 = NULL;
    idcu_CoroutineConfig config = {0};
    config.stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE;
    
    result = idcu_coro_create(scheduler, &coro1, test_coroutine_func, &count1, &config);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to create coroutine 1");
    
    result = idcu_coro_create(scheduler, &coro2, test_coroutine_func, &count2, &config);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to create coroutine 2");
    
    result = idcu_coro_scheduler_run(scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to run scheduler");
    
    TEST_ASSERT(test_counter == 7, "Expected counter to be 7, got %d", test_counter);
    
    result = idcu_coro_scheduler_destroy(scheduler);
    TEST_ASSERT(result == IDCU_SUCCESS, "Failed to destroy scheduler");
    
    TEST_PASS();
}

int main() {
    RUN_TEST(coroutine_create);
    RUN_TEST(coroutine_scheduler_run);
    RUN_TEST(coroutine_multiple);
    
    PRINT_TEST_RESULTS();
    return 0;
}
