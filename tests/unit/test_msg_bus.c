#include "test/test_framework.h"
#include "msg_bus.h"
#include "idcu/log/log.h"
#include "idcu/common/error_code.h"
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
    idcu_ctx_init(&ctx, 1, 1001);
    int test_value = 42;
    memcpy(ctx.data, &test_value, sizeof(test_value));
    ctx.len = sizeof(test_value);
    
    int ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_msg_send should succeed");
    IDCU_TEST_ASSERT(idcu_msg_get_count(&bus) == 1, "Count should be 1 after send");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_msg_recv should succeed");
    int received_value = 0;
    memcpy(&received_value, msg.data.data, sizeof(received_value));
    IDCU_TEST_ASSERT(received_value == 42, "Received value should be 42");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_msg_priority(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_StackContext ctx_low, ctx_norm, ctx_high, ctx_realtime;
    int val1 = 1, val2 = 2, val3 = 3, val4 = 4;
    
    idcu_ctx_init(&ctx_low, 1, 2001);
    memcpy(ctx_low.data, &val1, sizeof(val1));
    ctx_low.len = sizeof(val1);
    
    idcu_ctx_init(&ctx_norm, 1, 2002);
    memcpy(ctx_norm.data, &val2, sizeof(val2));
    ctx_norm.len = sizeof(val2);
    
    idcu_ctx_init(&ctx_high, 1, 2003);
    memcpy(ctx_high.data, &val3, sizeof(val3));
    ctx_high.len = sizeof(val3);
    
    idcu_ctx_init(&ctx_realtime, 1, 2004);
    memcpy(ctx_realtime.data, &val4, sizeof(val4));
    ctx_realtime.len = sizeof(val4);
    
    int ret;
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_LOW, &ctx_low);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Send low priority should succeed");
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx_norm);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Send normal priority should succeed");
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_HIGH, &ctx_high);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Send high priority should succeed");
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_REALTIME, &ctx_realtime);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Send realtime priority should succeed");
    
    IDCU_TEST_ASSERT(idcu_msg_get_count(&bus) == 4, "Count should be 4");
    
    idcu_Message msg;
    int received;
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "First recv should succeed");
    memcpy(&received, msg.data.data, sizeof(received));
    IDCU_TEST_ASSERT(received == 4, "First should be realtime (4)");
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Second recv should succeed");
    memcpy(&received, msg.data.data, sizeof(received));
    IDCU_TEST_ASSERT(received == 3, "Second should be high (3)");
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Third recv should succeed");
    memcpy(&received, msg.data.data, sizeof(received));
    IDCU_TEST_ASSERT(received == 2, "Third should be normal (2)");
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Fourth recv should succeed");
    memcpy(&received, msg.data.data, sizeof(received));
    IDCU_TEST_ASSERT(received == 1, "Fourth should be low (1)");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_msg_zerocopy(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    uint8_t test_data[] = "Hello Zero Copy!";
    uint32_t data_size = sizeof(test_data);
    
    int ret = idcu_msg_send_zerocopy(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, test_data, data_size);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_msg_send_zerocopy should succeed");
    
    idcu_Message msg;
    ret = idcu_msg_recv_zerocopy(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_msg_recv_zerocopy should succeed");
    IDCU_TEST_ASSERT(msg.payload != NULL, "Payload should not be NULL");
    IDCU_TEST_ASSERT(msg.payload->size == data_size, "Payload size should match");
    IDCU_TEST_ASSERT(memcmp(msg.payload->data, test_data, data_size) == 0, "Payload data should match");
    
    idcu_msg_release_payload(&bus, msg.payload);
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_msg_batch(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_MessageBatch batch;
    batch.count = 5;
    
    for (uint32_t i = 0; i < batch.count; i++) {
        idcu_StackContext ctx;
        int val = (int)(i + 1);
        idcu_ctx_init(&ctx, 1, 3000 + i);
        memcpy(ctx.data, &val, sizeof(val));
        ctx.len = sizeof(val);
        
        batch.msgs[i].data = ctx;
        batch.msgs[i].source_mod_id = 1;
        batch.msgs[i].target_mod_id = 2;
        batch.msgs[i].priority = IDCU_MSG_PRIO_NORMAL;
        batch.msgs[i].payload = NULL;
    }
    
    int ret = idcu_msg_send_batch(&bus, &batch);
    IDCU_TEST_ASSERT(ret == 5, "Should send 5 messages");
    IDCU_TEST_ASSERT(idcu_msg_get_count(&bus) == 5, "Count should be 5");
    
    idcu_MessageBatch recv_batch;
    ret = idcu_msg_recv_batch(&bus, 2, &recv_batch, 10);
    IDCU_TEST_ASSERT(ret == 5, "Should receive 5 messages");
    IDCU_TEST_ASSERT(recv_batch.count == 5, "Batch count should be 5");
    
    for (uint32_t i = 0; i < recv_batch.count; i++) {
        int val = 0;
        memcpy(&val, recv_batch.msgs[i].data.data, sizeof(val));
        IDCU_TEST_ASSERT(val == (int)(i + 1), "Message value should match");
    }
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_msg_error_handling(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    int ret = idcu_msg_send(NULL, 1, 2, IDCU_MSG_PRIO_NORMAL, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "NULL params should return error");
    
    idcu_StackContext ctx;
    idcu_ctx_init(&ctx, 1, 4001);
    ret = idcu_msg_send(&bus, 1, 2, (idcu_MsgPriority)999, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "Invalid priority should return error");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_QUEUE_EMPTY, "Empty queue should return error");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_msg_broadcast(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_StackContext ctx;
    int val = 100;
    idcu_ctx_init(&ctx, 1, 5001);
    memcpy(ctx.data, &val, sizeof(val));
    ctx.len = sizeof(val);
    
    int ret = idcu_msg_broadcast(&bus, 1, IDCU_MSG_PRIO_HIGH, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_msg_broadcast should succeed");
    
    idcu_Message msg1;
    ret = idcu_msg_recv(&bus, 100, &msg1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Module 100 should receive broadcast");
    int received_val = 0;
    memcpy(&received_val, msg1.data.data, sizeof(received_val));
    IDCU_TEST_ASSERT(received_val == 100, "Broadcast value should be 100");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Message Bus Tests");
    
    idcu_test_suite_add_test(&g_suite, "msg_bus_init_destroy", test_msg_bus_init_destroy);
    idcu_test_suite_add_test(&g_suite, "msg_send_recv", test_msg_send_recv);
    idcu_test_suite_add_test(&g_suite, "msg_priority", test_msg_priority);
    idcu_test_suite_add_test(&g_suite, "msg_zerocopy", test_msg_zerocopy);
    idcu_test_suite_add_test(&g_suite, "msg_batch", test_msg_batch);
    idcu_test_suite_add_test(&g_suite, "msg_error_handling", test_msg_error_handling);
    idcu_test_suite_add_test(&g_suite, "msg_broadcast", test_msg_broadcast);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
