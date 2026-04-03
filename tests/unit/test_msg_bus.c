#include "test/test_framework.h"
#include "scheduler/msg_bus.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_msg_bus_init_destroy(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    IDCU_TEST_ASSERT(idcu_msg_get_count(&bus) == 0, "Initial count should be 0");
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_msg_send_recv(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = IDCU_STACK_DATA_INT;
    ctx.data.int_val = 42;
    
    int ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_msg_send should succeed");
    IDCU_TEST_ASSERT(idcu_msg_get_count(&bus) == 1, "Count should be 1 after send");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_msg_recv should succeed");
    IDCU_TEST_ASSERT(msg.data.data.int_val == 42, "Received value should be 42");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_msg_broadcast(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = IDCU_STACK_DATA_INT;
    ctx.data.int_val = 100;
    
    int ret = idcu_msg_broadcast(&bus, 1, IDCU_MSG_PRIO_HIGH, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_msg_broadcast should succeed");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Message Bus Tests");
    
    idcu_test_suite_add_test(&g_suite, "msg_bus_init_destroy", test_msg_bus_init_destroy);
    idcu_test_suite_add_test(&g_suite, "msg_send_recv", test_msg_send_recv);
    idcu_test_suite_add_test(&g_suite, "msg_broadcast", test_msg_broadcast);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
