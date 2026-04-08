#include "idcu/common/error_code.h"
#include "idcu/log/log.h"
#include "idcu/msgbus/msg_bus.h"
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
    if (ret != 0) {
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
    if (ret != 0) {
        printf("test_msg_send_recv: FAIL - idcu_msg_recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    int received_value = 0;
    memcpy(&received_value, msg.data.data, sizeof(received_value));
    if (received_value != 42) {
        printf("test_msg_send_recv: FAIL - Received value should be 42, got %d\n", received_value);
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
    if (ret != 0) {
        printf("test_msg_priority: FAIL - Send low priority should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx_norm);
    if (ret != 0) {
        printf("test_msg_priority: FAIL - Send normal priority should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_HIGH, &ctx_high);
    if (ret != 0) {
        printf("test_msg_priority: FAIL - Send high priority should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_REALTIME, &ctx_realtime);
    if (ret != 0) {
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
    if (ret != 0) {
        printf("test_msg_priority: FAIL - First recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 4) {
        printf("test_msg_priority: FAIL - First should be realtime (4), got %d\n", received);
        idcu_msg_bus_destroy(&bus);
        return;
    }

    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != 0) {
        printf("test_msg_priority: FAIL - Second recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 3) {
        printf("test_msg_priority: FAIL - Second should be high (3), got %d\n", received);
        idcu_msg_bus_destroy(&bus);
        return;
    }

    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != 0) {
        printf("test_msg_priority: FAIL - Third recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 2) {
        printf("test_msg_priority: FAIL - Third should be normal (2), got %d\n", received);
        idcu_msg_bus_destroy(&bus);
        return;
    }

    ret = idcu_msg_recv(&bus, 2, &msg);
    if (ret != 0) {
        printf("test_msg_priority: FAIL - Fourth recv should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    memcpy(&received, msg.data.data, sizeof(received));
    if (received != 1) {
        printf("test_msg_priority: FAIL - Fourth should be low (1), got %d\n", received);
        idcu_msg_bus_destroy(&bus);
        return;
    }

    idcu_msg_bus_destroy(&bus);
    printf("test_msg_priority: PASS\n");
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
    if (ret != 0) {
        printf("test_msg_broadcast: FAIL - idcu_msg_broadcast should succeed\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }

    idcu_Message msg1;
    ret = idcu_msg_recv(&bus, 100, &msg1);
    if (ret != 0) {
        printf("test_msg_broadcast: FAIL - Module 100 should receive broadcast\n");
        idcu_msg_bus_destroy(&bus);
        return;
    }
    int received_val = 0;
    memcpy(&received_val, msg1.data.data, sizeof(received_val));
    if (received_val != 100) {
        printf("test_msg_broadcast: FAIL - Broadcast value should be 100, got %d\n", received_val);
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
    test_msg_broadcast();

    printf("=== All Tests Completed ===\n");

    return 0;
}
