#include <idcu/testframework/testframework.h>
#include <idcu/module_system/module_system.h>
#include <idcu/common/error_code.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_test_module_init_called = 0;
static int g_test_module_start_called = 0;
static int g_test_module_stop_called = 0;
static int g_test_module_destroy_called = 0;

static idcu_ErrorCode test_module_init(void) {
    g_test_module_init_called++;
    return IDCU_ERR_OK;
}

static idcu_ErrorCode test_module_start(void) {
    g_test_module_start_called++;
    return IDCU_ERR_OK;
}

static idcu_ErrorCode test_module_stop(void) {
    g_test_module_stop_called++;
    return IDCU_ERR_OK;
}

static idcu_ErrorCode test_module_destroy(void) {
    g_test_module_destroy_called++;
    return IDCU_ERR_OK;
}

static const idcu_ModuleDef g_test_module_def = {
    .name = "test_module",
    .version_str = "1.0.0",
    .description = "Test module for integration testing",
    .category = "test",
    .init = test_module_init,
    .start = test_module_start,
    .stop = test_module_stop,
    .destroy = test_module_destroy,
    .dependencies = NULL,
    .dependency_count = 0
};

IDCU_TEST_CASE(module_system, init_destroy) {
    idcu_ModuleSystem system;
    int ret = idcu_module_system_init(&system);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_module_system_destroy(&system);
}

IDCU_TEST_CASE(module_system, register_module) {
    idcu_ModuleSystem system;
    idcu_module_system_init(&system);
    
    int ret = idcu_module_system_register(&system, &g_test_module_def);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_Module* module = NULL;
    ret = idcu_module_system_find_by_name(&system, "test_module", &module);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT(module != NULL);
    
    idcu_module_system_destroy(&system);
}

IDCU_TEST_CASE(module_system, init_start_stop_destroy) {
    g_test_module_init_called = 0;
    g_test_module_start_called = 0;
    g_test_module_stop_called = 0;
    g_test_module_destroy_called = 0;
    
    idcu_ModuleSystem system;
    idcu_module_system_init(&system);
    
    idcu_module_system_register(&system, &g_test_module_def);
    
    int ret = idcu_module_system_init_module(&system, "test_module");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, g_test_module_init_called);
    
    ret = idcu_module_system_start_module(&system, "test_module");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, g_test_module_start_called);
    
    ret = idcu_module_system_stop_module(&system, "test_module");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, g_test_module_stop_called);
    
    ret = idcu_module_system_destroy_module(&system, "test_module");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, g_test_module_destroy_called);
    
    idcu_module_system_destroy(&system);
}

IDCU_TEST_CASE(module_system, all_modules) {
    idcu_ModuleSystem system;
    idcu_module_system_init(&system);
    
    idcu_module_system_register(&system, &g_test_module_def);
    
    idcu_ModuleInfo* infos = NULL;
    size_t count = 0;
    int ret = idcu_module_system_get_all(&system, &infos, &count);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, count);
    IDCU_TEST_ASSERT(infos != NULL);
    
    if (infos) {
        IDCU_TEST_ASSERT_STRING_EQUAL("test_module", infos[0].name);
        free(infos);
    }
    
    idcu_module_system_destroy(&system);
}

IDCU_TEST_CASE(module_system, by_category) {
    idcu_ModuleSystem system;
    idcu_module_system_init(&system);
    
    idcu_module_system_register(&system, &g_test_module_def);
    
    idcu_ModuleInfo* infos = NULL;
    size_t count = 0;
    int ret = idcu_module_system_get_by_category(&system, "test", &infos, &count);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, count);
    
    if (infos) {
        free(infos);
    }
    
    idcu_module_system_destroy(&system);
}

IDCU_TEST_CASE(module_system, unregister) {
    idcu_ModuleSystem system;
    idcu_module_system_init(&system);
    
    idcu_module_system_register(&system, &g_test_module_def);
    
    int ret = idcu_module_system_unregister(&system, "test_module");
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_Module* module = NULL;
    ret = idcu_module_system_find_by_name(&system, "test_module", &module);
    IDCU_TEST_ASSERT_NOT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_module_system_destroy(&system);
}

#if 0
static int g_coro_count = 0;

static void test_coroutine_func(void* arg) {
    int* counter = (int*)arg;
    (*counter)++;
    g_coro_count++;
}

IDCU_TEST_CASE(coroutine, scheduler_init_destroy) {
    idcu_CoroutineScheduler* scheduler = NULL;
    int ret = idcu_coro_scheduler_init(&scheduler);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT(scheduler != NULL);
    
    idcu_coro_scheduler_destroy(scheduler);
}

IDCU_TEST_CASE(coroutine, create_destroy) {
    idcu_CoroutineScheduler* scheduler = NULL;
    idcu_coro_scheduler_init(&scheduler);
    
    idcu_Coroutine* coro = NULL;
    int counter = 0;
    idcu_CoroutineConfig config = {
        .stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE,
        .name = "test_coro"
    };
    
    int ret = idcu_coro_create(scheduler, &coro, test_coroutine_func, &counter, &config);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT(coro != NULL);
    
    idcu_coro_destroy(coro);
    idcu_coro_scheduler_destroy(scheduler);
}

IDCU_TEST_CASE(coroutine, get_name_state) {
    idcu_CoroutineScheduler* scheduler = NULL;
    idcu_coro_scheduler_init(&scheduler);
    
    idcu_Coroutine* coro = NULL;
    int counter = 0;
    idcu_CoroutineConfig config = {
        .stack_size = IDCU_COROUTINE_DEFAULT_STACK_SIZE,
        .name = "test_coro_with_name"
    };
    
    idcu_coro_create(scheduler, &coro, test_coroutine_func, &counter, &config);
    
    const char* name = idcu_coro_get_name(coro);
    IDCU_TEST_ASSERT_STRING_EQUAL("test_coro_with_name", name);
    
    idcu_CoroutineState state = idcu_coro_get_state(coro);
    IDCU_TEST_ASSERT(state == IDCU_CORO_READY || state == IDCU_CORO_SUSPENDED);
    
    idcu_coro_destroy(coro);
    idcu_coro_scheduler_destroy(scheduler);
}

static int g_msg_received = 0;
static idcu_MsgTopic g_last_topic = 0;
static const void* g_last_data = NULL;
static size_t g_last_data_size = 0;

static void test_msg_handler(idcu_MsgTopic topic, const void* data, size_t data_size, void* user_data) {
    g_msg_received++;
    g_last_topic = topic;
    g_last_data = data;
    g_last_data_size = data_size;
}

IDCU_TEST_CASE(msgbus, init_destroy) {
    idcu_MsgBus* bus = NULL;
    int ret = idcu_msgbus_init(&bus);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT(bus != NULL);
    
    idcu_msgbus_destroy(bus);
}

IDCU_TEST_CASE(msgbus, subscribe_unsubscribe) {
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    idcu_MsgSubscriber* subscriber = NULL;
    int ret = idcu_msgbus_subscribe(bus, 1, test_msg_handler, NULL, &subscriber);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT(subscriber != NULL);
    
    ret = idcu_msgbus_unsubscribe(bus, subscriber);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_msgbus_destroy(bus);
}

IDCU_TEST_CASE(msgbus, publish_process) {
    g_msg_received = 0;
    g_last_topic = 0;
    g_last_data = NULL;
    g_last_data_size = 0;
    
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    idcu_MsgSubscriber* subscriber = NULL;
    idcu_msgbus_subscribe(bus, 100, test_msg_handler, NULL, &subscriber);
    
    int test_data = 42;
    int ret = idcu_msgbus_publish(bus, 100, &test_data, sizeof(test_data), IDCU_MSG_PRIORITY_NORMAL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    ret = idcu_msgbus_process(bus);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    IDCU_TEST_ASSERT_EQUAL(1, g_msg_received);
    IDCU_TEST_ASSERT_EQUAL(100, g_last_topic);
    IDCU_TEST_ASSERT(g_last_data != NULL);
    IDCU_TEST_ASSERT_EQUAL(sizeof(test_data), g_last_data_size);
    IDCU_TEST_ASSERT_EQUAL(42, *(int*)g_last_data);
    
    idcu_msgbus_unsubscribe(bus, subscriber);
    idcu_msgbus_destroy(bus);
}

IDCU_TEST_CASE(msgbus, multiple_priorities) {
    g_msg_received = 0;
    
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    idcu_MsgSubscriber* subscriber = NULL;
    idcu_msgbus_subscribe(bus, 200, test_msg_handler, NULL, &subscriber);
    
    int data = 1;
    idcu_msgbus_publish(bus, 200, &data, sizeof(data), IDCU_MSG_PRIORITY_LOW);
    idcu_msgbus_publish(bus, 200, &data, sizeof(data), IDCU_MSG_PRIORITY_HIGH);
    idcu_msgbus_publish(bus, 200, &data, sizeof(data), IDCU_MSG_PRIORITY_CRITICAL);
    
    idcu_msgbus_process(bus);
    IDCU_TEST_ASSERT_EQUAL(3, g_msg_received);
    
    idcu_msgbus_unsubscribe(bus, subscriber);
    idcu_msgbus_destroy(bus);
}

IDCU_TEST_CASE(integration, module_and_msgbus) {
    idcu_ModuleSystem module_system;
    idcu_module_system_init(&module_system);
    idcu_module_system_register(&module_system, &g_test_module_def);
    
    idcu_MsgBus* bus = NULL;
    idcu_msgbus_init(&bus);
    
    idcu_Module* module = NULL;
    idcu_module_system_find_by_name(&module_system, "test_module", &module);
    IDCU_TEST_ASSERT(module != NULL);
    
    idcu_MsgSubscriber* subscriber = NULL;
    idcu_msgbus_subscribe(bus, 999, test_msg_handler, NULL, &subscriber);
    
    int data = 99;
    idcu_msgbus_publish(bus, 999, &data, sizeof(data), IDCU_MSG_PRIORITY_NORMAL);
    idcu_msgbus_process(bus);
    IDCU_TEST_ASSERT_EQUAL(1, g_msg_received);
    
    idcu_msgbus_unsubscribe(bus, subscriber);
    idcu_msgbus_destroy(bus);
    idcu_module_system_destroy(&module_system);
}
#endif

int main(void) {
    return idcu_test_run_all();
}
