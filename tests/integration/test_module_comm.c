#include "test_framework.h"
#include "msg_bus.h"
#include "module_def.h"
#include "module_registry.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static const ModuleMeta g_sender_meta = {
    .name = "sender_module",
    .version = "1.0.0",
    .author = "test",
    .description = "Test sender module"
};

static const ModuleMeta g_receiver_meta = {
    .name = "receiver_module",
    .version = "1.0.0",
    .author = "test",
    .description = "Test receiver module"
};

static int g_received_messages = 0;

static int sender_init(void* data) { return ERR_OK; }
static int sender_run(void* data) { return ERR_OK; }
static int sender_stop(void* data) { return ERR_OK; }

static int receiver_init(void* data) { return ERR_OK; }
static int receiver_run(void* data) { return ERR_OK; }
static int receiver_stop(void* data) { return ERR_OK; }

static const ModuleInterface g_sender_iface = {
    .meta = &g_sender_meta,
    .init = sender_init,
    .run = sender_run,
    .stop = sender_stop,
    .priority = 100,
    .user_data = NULL
};

static const ModuleInterface g_receiver_iface = {
    .meta = &g_receiver_meta,
    .init = receiver_init,
    .run = receiver_run,
    .stop = receiver_stop,
    .priority = 50,
    .user_data = NULL
};

static void test_module_registration(void) {
    ModuleRegistry reg;
    int ret = registry_init(&reg);
    TEST_ASSERT(ret == ERR_OK, "registry_init should succeed");
    
    ret = registry_register(&reg, &g_sender_iface);
    TEST_ASSERT(ret == ERR_OK, "register sender should succeed");
    
    ret = registry_register(&reg, &g_receiver_iface);
    TEST_ASSERT(ret == ERR_OK, "register receiver should succeed");
    
    TEST_ASSERT(registry_get_count(&reg) == 2, "Should have 2 modules registered");
    
    registry_clear(&reg);
    TEST_PASS();
}

static void test_message_send_between_modules(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    
    StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = STACK_DATA_INT;
    ctx.data.int_val = 12345;
    
    int ret = msg_send(&bus, 1, 2, MSG_PRIO_NORMAL, &ctx);
    TEST_ASSERT(ret == ERR_OK, "msg_send should succeed");
    
    Message msg;
    ret = msg_recv(&bus, 2, &msg);
    TEST_ASSERT(ret == ERR_OK, "msg_recv should succeed");
    TEST_ASSERT(msg.source_mod_id == 1, "Source module ID should be 1");
    TEST_ASSERT(msg.target_mod_id == 2, "Target module ID should be 2");
    TEST_ASSERT(msg.data.int_val == 12345, "Data value should be 12345");
    
    msg_release(&bus, &msg);
    msg_bus_destroy(&bus);
    TEST_PASS();
}

static void test_message_priority_order(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    
    StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = STACK_DATA_INT;
    
    ctx.data.int_val = 1;
    msg_send(&bus, 1, 2, MSG_PRIO_LOW, &ctx);
    
    ctx.data.int_val = 3;
    msg_send(&bus, 1, 2, MSG_PRIO_REALTIME, &ctx);
    
    ctx.data.int_val = 2;
    msg_send(&bus, 1, 2, MSG_PRIO_HIGH, &ctx);
    
    TEST_ASSERT(msg_get_count(&bus) == 3, "Should have 3 messages in bus");
    
    msg_bus_destroy(&bus);
    TEST_PASS();
}

static void test_broadcast_message(void) {
    MessageBus bus;
    msg_bus_init(&bus);
    
    StackContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.type = STACK_DATA_STRING;
    strncpy(ctx.data.string_val, "broadcast test", sizeof(ctx.data.string_val) - 1);
    
    int ret = msg_broadcast(&bus, 1, MSG_PRIO_NORMAL, &ctx);
    TEST_ASSERT(ret == ERR_OK, "msg_broadcast should succeed");
    
    msg_bus_destroy(&bus);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Module Communication Integration Tests");
    
    test_suite_add_test(&g_suite, "module_registration", test_module_registration);
    test_suite_add_test(&g_suite, "message_send_between_modules", test_message_send_between_modules);
    test_suite_add_test(&g_suite, "message_priority_order", test_message_priority_order);
    test_suite_add_test(&g_suite, "broadcast_message", test_broadcast_message);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
