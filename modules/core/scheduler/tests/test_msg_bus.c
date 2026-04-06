#include "../include/msg_bus.h"
#include "idcu/log/log.h"
#include "idcu/common/error_code.h"
#include <stdio.h>
#include <string.h>

static void test_msg_bus_init_destroy(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    if (idcu_msg_get_count(&bus) != 0) {
        printf("test_msg_bus_init_destroy: FAIL - Initial count should be 0\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_msg_bus_destroy(&bus);
    printf("test_msg_bus_init_destroy: PASS\n");
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
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_send_recv: FAIL - idcu_msg_send should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    if (idcu_msg_get_count(&bus) != 1) {
        printf("test_msg_send_recv: FAIL - Count should be 1 after send\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_send_recv: FAIL - idcu_msg_recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    int received_value = 0;
    memcpy(&received_value, msg.data.data, sizeof(received_value));
    if (received_value != 42) {
        printf("test_msg_send_recv: FAIL - Received value should be 42\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_msg_bus_destroy(&bus);
    printf("test_msg_send_recv: PASS\n");
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
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - Send low priority should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx_norm);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - Send normal priority should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_HIGH, &ctx_high);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - Send high priority should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_REALTIME, &ctx_realtime);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - Send realtime priority should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    if (idcu_msg_get_count(&bus) != 4) {
        printf("test_msg_priority: FAIL - Count should be 4\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_Message msg;
    int received;
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - First recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 4) {
        printf("test_msg_priority: FAIL - First should be realtime (4)\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - Second recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 3) {
        printf("test_msg_priority: FAIL - Second should be high (3)\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - Third recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 2) {
        printf("test_msg_priority: FAIL - Third should be normal (2)\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_priority: FAIL - Fourth recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 1) {
        printf("test_msg_priority: FAIL - Fourth should be low (1)\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_msg_bus_destroy(&bus);
    printf("test_msg_priority: PASS\n");
}

static void test_msg_zerocopy(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    uint8_t test_data[] = "Hello Zero Copy!";
    uint32_t data_size = sizeof(test_data);
    
    int ret = idcu_msg_send_zerocopy(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, test_data, data_size);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_zerocopy: FAIL - idcu_msg_send_zerocopy should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_Message msg;
    ret = idcu_msg_recv_zerocopy(&bus, 2, &msg);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_zerocopy: FAIL - idcu_msg_recv_zerocopy should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    if (msg.payload == NULL) {
        printf("test_msg_zerocopy: FAIL - Payload should not be NULL\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    if (msg.payload->size != data_size) {
        printf("test_msg_zerocopy: FAIL - Payload size should match\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    if (memcmp(msg.payload->data, test_data, data_size) != 0) {
        printf("test_msg_zerocopy: FAIL - Payload data should match\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_msg_release_payload(&bus, msg.payload);
    idcu_msg_bus_destroy(&bus);
    printf("test_msg_zerocopy: PASS\n");
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
    if (ret != 5) {
        printf("test_msg_batch: FAIL - Should send 5 messages\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    if (idcu_msg_get_count(&bus) != 5) {
        printf("test_msg_batch: FAIL - Count should be 5\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_MessageBatch recv_batch;
    ret = idcu_msg_recv_batch(&bus, 2, &recv_batch, 10);
    if (ret != 5) {
        printf("test_msg_batch: FAIL - Should receive 5 messages\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    if (recv_batch.count != 5) {
        printf("test_msg_batch: FAIL - Batch count should be 5\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    for (uint32_t i = 0; i < recv_batch.count; i++) {
        int val = 0;
        memcpy(&val, recv_batch.msgs[i].data.data, sizeof(val));
        if (val != (int)(i + 1)) {
            printf("test_msg_batch: FAIL - Message value should match\n");
            idcu_msg_bus_destroy(&bus);
            return;
        }
    }
    
    idcu_msg_bus_destroy(&bus);
    printf("test_msg_batch: PASS\n");
}

static void test_msg_error_handling(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    int ret = idcu_msg_send(NULL, 1, 2, IDCU_MSG_PRIO_NORMAL, NULL);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_msg_error_handling: FAIL - NULL params should return error\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_StackContext ctx;
    idcu_ctx_init(&ctx, 1, 4001);
    ret = idcu_msg_send(&bus, 1, 2, (idcu_MsgPriority)999, &ctx);
    if (ret != IDCU_ERR_INVALID_PARAM) {
        printf("test_msg_error_handling: FAIL - Invalid priority should return error\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != IDCU_ERR_QUEUE_EMPTY) {
        printf("test_msg_error_handling: FAIL - Empty queue should return error\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_msg_bus_destroy(&bus);
    printf("test_msg_error_handling: PASS\n");
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
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_broadcast: FAIL - idcu_msg_broadcast should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_Message msg1;
    ret = idcu_msg_recv(&bus, 100, &msg1);
    if (ret != IDCU_ERR_SUCCESS) {
        printf("test_msg_broadcast: FAIL - Module 100 should receive broadcast\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    int received_val = 0;
    memcpy(&received_val, msg1.data.data, sizeof(received_val));
    if (received_val != 100) {
        printf("test_msg_broadcast: FAIL - Broadcast value should be 100\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    
    idcu_msg_bus_destroy(&bus);
    printf("test_msg_broadcast: PASS\n");
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    printf("=== Message Bus Tests ===\n");
    
    test_msg_bus_init_destroy();
    test_msg_send_recv();
    test_msg_priority();
    test_msg_zerocopy();
    test_msg_batch();
    test_msg_error_handling();
    test_msg_broadcast();
    
    printf("=== All Tests Completed ===\n");
    
    return 0;
}
