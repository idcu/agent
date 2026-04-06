#include "test_framework.h"
#include "micro_kernel.h"
#include "module_def.h"
#include "module_registry.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_MicroKernel g_kernel;

idcu_MicroKernel* idcu_get_kernel(void)
{
    return &g_kernel;
}

static idcu_TestSuite g_suite;

static int test_mod1_init(void) { return IDCU_ERR_OK; }
static int test_mod1_run(void) { return IDCU_ERR_OK; }
static int test_mod1_stop(void) { return IDCU_ERR_OK; }

static int test_mod2_init(void) { return IDCU_ERR_OK; }
static int test_mod2_run(void) { return IDCU_ERR_OK; }
static int test_mod2_stop(void) { return IDCU_ERR_OK; }

static const idcu_ModuleInterface g_test_mod1 = {
    .name = "test_mod1",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_mod1_init,
    .run = test_mod1_run,
    .stop = test_mod1_stop
};

static const idcu_ModuleInterface g_test_mod2 = {
    .name = "test_mod2",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_mod2_init,
    .run = test_mod2_run,
    .stop = test_mod2_stop
};

static void test_micro_kernel_init_destroy(void) {
    idcu_MicroKernel kernel;
    idcu_kernel_init(&kernel);
    
    IDCU_TEST_ASSERT(kernel.should_exit == 0, "should_exit should be 0 after init");
    IDCU_TEST_ASSERT(kernel.sb_cnt == 0, "sb_cnt should be 0 after init");
    IDCU_TEST_ASSERT(kernel.tracked_cnt == 0, "tracked_cnt should be 0 after init");
    
    idcu_kernel_stop(&kernel);
    IDCU_TEST_PASS();
}

static void test_micro_kernel_message_bus(void) {
    idcu_MicroKernel kernel;
    idcu_kernel_init(&kernel);
    
    idcu_StackContext ctx;
    idcu_ctx_init(&ctx, 1, 1001);
    int test_value = 42;
    memcpy(ctx.data, &test_value, sizeof(test_value));
    ctx.len = sizeof(test_value);
    
    int ret = idcu_msg_send(&kernel.msg, 1, 2, IDCU_MSG_PRIO_NORMAL, &ctx);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "msg_send should succeed");
    
    idcu_Message msg;
    ret = idcu_msg_recv(&kernel.msg, 2, &msg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "msg_recv should succeed");
    IDCU_TEST_ASSERT(msg.source_mod_id == 1, "Source module ID should be 1");
    IDCU_TEST_ASSERT(msg.target_mod_id == 2, "Target module ID should be 2");
    
    int received_value = 0;
    memcpy(&received_value, msg.data.data, sizeof(received_value));
    IDCU_TEST_ASSERT(received_value == 42, "Data value should be 42");
    
    idcu_kernel_stop(&kernel);
    IDCU_TEST_PASS();
}

static void test_micro_kernel_module_lifecycle(void) {
    idcu_MicroKernel kernel;
    idcu_kernel_init(&kernel);
    
    idcu_kernel_start_modules(&kernel);
    
    IDCU_TEST_ASSERT(kernel.tracked_cnt > 0, "Should have tracked modules");
    
    idcu_kernel_stop(&kernel);
    
    for (uint32_t i = 0; i < kernel.tracked_cnt; i++) {
        IDCU_TEST_ASSERT(kernel.tracked_modules[i].state == IDCU_MOD_STATE_STOPPED, 
                    "All modules should be stopped");
    }
    
    IDCU_TEST_PASS();
}

static void test_micro_kernel_signal_handler_setup(void) {
    idcu_MicroKernel kernel;
    idcu_kernel_init(&kernel);
    
    idcu_kernel_set_signal_handler(&kernel);
    
    idcu_kernel_stop(&kernel);
    IDCU_TEST_PASS();
}

static void test_micro_kernel_with_registry(void) {
    idcu_ModuleRegistry reg;
    idcu_MicroKernel kernel;
    
    idcu_module_registry_init(&reg);
    idcu_kernel_init(&kernel);
    
    int ret = idcu_module_registry_register(&reg, &g_test_mod1, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register test_mod1 should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_mod2, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register test_mod2 should succeed");
    
    ret = idcu_module_registry_init_all(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "init_all should succeed");
    
    ret = idcu_module_registry_run_all(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "run_all should succeed");
    
    idcu_module_registry_stop_all(&reg);
    idcu_module_registry_destroy(&reg);
    idcu_kernel_stop(&kernel);
    
    IDCU_TEST_PASS();
}

static void test_micro_kernel_sandbox_setup(void) {
    idcu_MicroKernel kernel;
    idcu_kernel_init(&kernel);
    
    idcu_kernel_start_modules(&kernel);
    
    for (uint32_t i = 0; i < kernel.sb_cnt; i++) {
        IDCU_TEST_ASSERT((kernel.sandbox[i].perm & IDCU_PERM_SEND) != 0, 
                    "Should have send permission");
        IDCU_TEST_ASSERT((kernel.sandbox[i].perm & IDCU_PERM_RECV) != 0, 
                    "Should have recv permission");
        IDCU_TEST_ASSERT((kernel.sandbox[i].perm & IDCU_PERM_RUN) != 0, 
                    "Should have run permission");
    }
    
    idcu_kernel_stop(&kernel);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Micro Kernel Integration Tests");
    
    idcu_test_suite_add_test(&g_suite, "micro_kernel_init_destroy", test_micro_kernel_init_destroy);
    idcu_test_suite_add_test(&g_suite, "micro_kernel_message_bus", test_micro_kernel_message_bus);
    idcu_test_suite_add_test(&g_suite, "micro_kernel_module_lifecycle", test_micro_kernel_module_lifecycle);
    idcu_test_suite_add_test(&g_suite, "micro_kernel_signal_handler_setup", test_micro_kernel_signal_handler_setup);
    idcu_test_suite_add_test(&g_suite, "micro_kernel_with_registry", test_micro_kernel_with_registry);
    idcu_test_suite_add_test(&g_suite, "micro_kernel_sandbox_setup", test_micro_kernel_sandbox_setup);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
