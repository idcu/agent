#include "test_framework.h"
#include "msg_bus.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_msg_bus_init_destroy(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    TEST_ASSERT(msg_get_count(&bus) == 0, "Initial count should be 0");
    msg_bus_destroy(&bus);
    TEST_PASS();
}

static void test_msg_send_recv(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    
    StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = STACK_DATA_INT;
    ctx.data.int_val = 42;
    
    int ret = msg_send(&bus, 1, 2, MSG_PRIO_NORMAL, &ctx);
    TEST_ASSERT(ret == ERR_OK, "msg_send should succeed");
    TEST_ASSERT(msg_get_count(&bus) == 1, "Count should be 1 after send");
    
    Message msg;
    ret = msg_recv(&bus, 2, &msg);
    TEST_ASSERT(ret == ERR_OK, "msg_recv should succeed");
    TEST_ASSERT(msg.data.int_val == 42, "Received value should be 42");
    
    msg_release(&bus, &msg);
    TEST_ASSERT(msg_get_count(&bus) == 0, "Count should be 0 after release");
    
    msg_bus_destroy(&bus);
    TEST_PASS();
}

static void test_msg_broadcast(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    
    StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = STACK_DATA_INT;
    ctx.data.int_val = 100;
    
    int ret = msg_broadcast(&bus, 1, MSG_PRIO_HIGH, &ctx);
    TEST_ASSERT(ret == ERR_OK, "msg_broadcast should succeed");
    
    msg_bus_destroy(&bus);
    TEST_PASS();
}

static void test_msg_priority(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    
    StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = STACK_DATA_INT;
    
    ctx.data.int_val = 1;
    msg_send(&bus, 1, 2, MSG_PRIO_LOW, &ctx);
    
    ctx.data.int_val = 3;
    msg_send(&bus, 1, 2, MSG_PRIO_HIGH, &ctx);
    
    ctx.data.int_val = 2;
    msg_send(&bus, 1, 2, MSG_PRIO_NORMAL, &ctx);
    
    TEST_ASSERT(msg_get_count(&bus) == 3, "Should have 3 messages");
    
    msg_bus_destroy(&bus);
    TEST_PASS();
}

static void test_msg_batch(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    
    MessageBatch batch;
    msg_batch_init(&batch, MSG_PRIO_NORMAL);
    
    StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = STACK_DATA_INT;
    
    for (int i = 0; i < 5; i++) {
        ctx.data.int_val = i;
        msg_batch_add(&batch, 1, 2, &ctx);
    }
    
    int ret = msg_send_batch(&bus, &batch);
    TEST_ASSERT(ret == ERR_OK, "msg_send_batch should succeed");
    TEST_ASSERT(msg_get_count(&bus) == 5, "Should have 5 messages after batch send");
    
    msg_bus_destroy(&bus);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Message Bus Tests");
    
    test_suite_add_test(&g_suite, "msg_bus_init_destroy", test_msg_bus_init_destroy);
    test_suite_add_test(&g_suite, "msg_send_recv", test_msg_send_recv);
    test_suite_add_test(&g_suite, "msg_broadcast", test_msg_broadcast);
    test_suite_add_test(&g_suite, "msg_priority", test_msg_priority);
    test_suite_add_test(&g_suite, "msg_batch", test_msg_batch);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
