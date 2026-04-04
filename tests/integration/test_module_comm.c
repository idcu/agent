#include "test_framework.h"
#include "micro_kernel.h"
#include "msg_bus.h"
#include "module_def.h"
#include "module_registry.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static idcu_MicroKernel g_kernel;

idcu_MicroKernel* idcu_get_kernel(void)
{
    return &g_kernel;
}

static idcu_TestSuite g_suite;

typedef struct {
    idcu_MessageBus* bus;
    uint32_t sender_id;
    uint32_t receiver_id;
    int message_count;
    int received_count;
    int last_received_value;
} CommTestData;

static CommTestData g_comm_data;

static int test_sender_init(void) {
    return IDCU_ERR_SUCCESS;
}

static int test_sender_run(void) {
    idcu_StackContext ctx;
    idcu_ctx_init(&ctx, g_comm_data.sender_id, g_comm_data.message_count);
    int val = g_comm_data.message_count++;
    memcpy(ctx.data, &val, sizeof(val));
    ctx.len = sizeof(val);
    
    int ret = idcu_msg_send(g_comm_data.bus, g_comm_data.sender_id, 
                            g_comm_data.receiver_id, IDCU_MSG_PRIO_NORMAL, &ctx);
    return ret;
}

static int test_sender_stop(void) {
    return IDCU_ERR_SUCCESS;
}

static int test_receiver_init(void) {
    g_comm_data.received_count = 0;
    g_comm_data.last_received_value = -1;
    return IDCU_ERR_SUCCESS;
}

static int test_receiver_run(void) {
    idcu_Message msg;
    int ret = idcu_msg_recv(g_comm_data.bus, g_comm_data.receiver_id, &msg);
    if (ret == IDCU_ERR_SUCCESS) {
        g_comm_data.received_count++;
        memcpy(&g_comm_data.last_received_value, msg.data.data, sizeof(int));
    }
    return IDCU_ERR_SUCCESS;
}

static int test_receiver_stop(void) {
    return IDCU_ERR_SUCCESS;
}

static const idcu_ModuleInterface g_test_sender = {
    .name = "test_sender",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_sender_init,
    .run = test_sender_run,
    .stop = test_sender_stop
};

static const idcu_ModuleInterface g_test_receiver = {
    .name = "test_receiver",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_receiver_init,
    .run = test_receiver_run,
    .stop = test_receiver_stop
};

static void test_module_registration(void) {
    idcu_ModuleRegistry reg;
    int ret = idcu_module_registry_init(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "registry_init should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_sender, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "register sender should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_receiver, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "register receiver should succeed");
    
    IDCU_TEST_ASSERT(idcu_module_registry_get_count(&reg) == 2, "Should have 2 modules registered");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_message_send_between_modules(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_StackContext ctx;
    idcu_ctx_init(&ctx, 1, 1001);
    int test_val = 12345;
    memcpy(ctx.data, &test_val, sizeof(test_val));
    ctx.len = sizeof(test_val);
    
    int ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "msg_send should succeed");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "msg_recv should succeed");
    IDCU_TEST_ASSERT(msg.source_mod_id == 1, "Source module ID should be 1");
    IDCU_TEST_ASSERT(msg.target_mod_id == 2, "Target module ID should be 2");
    
    int received_val = 0;
    memcpy(&received_val, msg.data.data, sizeof(received_val));
    IDCU_TEST_ASSERT(received_val == 12345, "Data value should be 12345");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_message_priority_order(void) {
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
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx_norm);
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_HIGH, &ctx_high);
    ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_REALTIME, &ctx_realtime);
    
    IDCU_TEST_ASSERT(idcu_msg_get_count(&bus) == 4, "Should have 4 messages in bus");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_broadcast_message(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    idcu_StackContext ctx;
    int val = 100;
    idcu_ctx_init(&ctx, 1, 5001);
    memcpy(ctx.data, &val, sizeof(val));
    ctx.len = sizeof(val);
    
    int ret = idcu_msg_broadcast(&bus, 1, IDCU_MSG_PRIO_HIGH, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "msg_broadcast should succeed");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_real_module_communication(void) {
    idcu_ModuleRegistry reg;
    idcu_MessageBus bus;
    
    idcu_module_registry_init(&reg);
    idcu_msg_bus_init(&bus);
    
    memset(&g_comm_data, 0, sizeof(g_comm_data));
    g_comm_data.bus = &bus;
    g_comm_data.message_count = 0;
    
    idcu_module_registry_register(&reg, &g_test_sender, IDCU_MOD_PRIO_NORMAL);
    idcu_module_registry_register(&reg, &g_test_receiver, IDCU_MOD_PRIO_NORMAL);
    
    const idcu_RegisteredModule* sender_mod = idcu_module_registry_find_by_name(&reg, "test_sender");
    const idcu_RegisteredModule* receiver_mod = idcu_module_registry_find_by_name(&reg, "test_receiver");
    IDCU_TEST_ASSERT(sender_mod != NULL, "sender module should be found");
    IDCU_TEST_ASSERT(receiver_mod != NULL, "receiver module should be found");
    
    g_comm_data.sender_id = sender_mod->module_id;
    g_comm_data.receiver_id = receiver_mod->module_id;
    
    idcu_module_registry_init_module(&reg, sender_mod->module_id);
    idcu_module_registry_init_module(&reg, receiver_mod->module_id);
    
    idcu_module_registry_run_module(&reg, sender_mod->module_id);
    
    for (int i = 0; i < 5; i++) {
        idcu_module_registry_run_module(&reg, sender_mod->module_id);
        idcu_module_registry_run_module(&reg, receiver_mod->module_id);
    }
    
    IDCU_TEST_ASSERT(g_comm_data.message_count > 0, "Should have sent some messages");
    
    idcu_module_registry_stop_module(&reg, sender_mod->module_id);
    idcu_module_registry_stop_module(&reg, receiver_mod->module_id);
    
    idcu_module_registry_destroy(&reg);
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

static void test_string_message(void) {
    idcu_MessageBus bus;
    idcu_msg_bus_init(&bus);
    
    const char* test_str = "Hello, IDCU!";
    idcu_StackContext ctx;
    idcu_ctx_init(&ctx, 1, 6001);
    strncpy((char*)ctx.data, test_str, sizeof(ctx.data) - 1);
    ctx.len = (uint16_t)strlen(test_str);
    
    int ret = idcu_msg_send(&bus, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "msg_send should succeed");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&bus, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "msg_recv should succeed");
    IDCU_TEST_ASSERT(strcmp((char*)msg.data.data, test_str) == 0, "String data should match");
    
    idcu_msg_bus_destroy(&bus);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Module Communication Integration Tests");
    
    idcu_test_suite_add_test(&g_suite, "module_registration", test_module_registration);
    idcu_test_suite_add_test(&g_suite, "message_send_between_modules", test_message_send_between_modules);
    idcu_test_suite_add_test(&g_suite, "message_priority_order", test_message_priority_order);
    idcu_test_suite_add_test(&g_suite, "broadcast_message", test_broadcast_message);
    idcu_test_suite_add_test(&g_suite, "real_module_communication", test_real_module_communication);
    idcu_test_suite_add_test(&g_suite, "string_message", test_string_message);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
