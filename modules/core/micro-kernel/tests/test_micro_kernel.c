#include "../../test-framework/include/test_framework.h"
#include "../include/micro_kernel.h"
#include "../../module-system/include/module_registry.h"
#include "../../module-system/include/module_def.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_MicroKernel g_kernel;

idcu_MicroKernel* idcu_get_kernel(void)
{
    return &g_kernel;
}

static idcu_TestSuite g_suite;

static int test_module1_init_called = 0;
static int test_module1_run_called = 0;
static int test_module1_stop_called = 0;

static int test_module1_init(void) {
    test_module1_init_called = 1;
    return IDCU_ERR_OK;
}

static int test_module1_run(void) {
    test_module1_run_called = 1;
    return IDCU_ERR_OK;
}

static int test_module1_stop(void) {
    test_module1_stop_called = 1;
    return IDCU_ERR_OK;
}

static int test_module2_init_called = 0;
static int test_module2_run_called = 0;
static int test_module2_stop_called = 0;

static int test_module2_init(void) {
    test_module2_init_called = 1;
    return IDCU_ERR_OK;
}

static int test_module2_run(void) {
    test_module2_run_called = 1;
    return IDCU_ERR_OK;
}

static int test_module2_stop(void) {
    test_module2_stop_called = 1;
    return IDCU_ERR_OK;
}

static const idcu_ModuleInterface g_test_iface1 = {
    .name = "test_module1",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_module1_init,
    .run = test_module1_run,
    .stop = test_module1_stop
};

static const idcu_ModuleInterface g_test_iface2 = {
    .name = "test_module2",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_module2_init,
    .run = test_module2_run,
    .stop = test_module2_stop
};

static void reset_test_flags(void) {
    test_module1_init_called = 0;
    test_module1_run_called = 0;
    test_module1_stop_called = 0;
    test_module2_init_called = 0;
    test_module2_run_called = 0;
    test_module2_stop_called = 0;
}

static void test_kernel_init(void) {
    idcu_MicroKernel k;
    
    idcu_kernel_init(&k);
    
    IDCU_TEST_ASSERT(k.should_exit == 0, "should_exit should be 0 after init");
    IDCU_TEST_ASSERT(k.sb_cnt == 0, "sb_cnt should be 0 after init");
    IDCU_TEST_ASSERT(k.tracked_cnt == 0, "tracked_cnt should be 0 after init");
    
    IDCU_TEST_PASS();
}

static void test_kernel_stop(void) {
    idcu_MicroKernel k;
    
    idcu_kernel_init(&k);
    
    k.tracked_cnt = 2;
    k.tracked_modules[0].iface = &g_test_iface1;
    k.tracked_modules[0].state = IDCU_MOD_STATE_RUNNING;
    k.tracked_modules[1].iface = &g_test_iface2;
    k.tracked_modules[1].state = IDCU_MOD_STATE_RUNNING;
    
    reset_test_flags();
    idcu_kernel_stop(&k);
    
    IDCU_TEST_ASSERT(k.tracked_modules[0].state == IDCU_MOD_STATE_STOPPED, "Module 0 should be stopped");
    IDCU_TEST_ASSERT(k.tracked_modules[1].state == IDCU_MOD_STATE_STOPPED, "Module 1 should be stopped");
    
    IDCU_TEST_PASS();
}

static void test_kernel_set_signal_handler(void) {
    idcu_MicroKernel k;
    
    idcu_kernel_init(&k);
    idcu_kernel_set_signal_handler(&k);
    
    IDCU_TEST_PASS();
}

static void test_kernel_start_stop_lifecycle(void) {
    idcu_MicroKernel k;
    idcu_ModuleRegistry reg;
    
    idcu_module_registry_init(&reg);
    
    reset_test_flags();
    idcu_module_registry_register(&reg, &g_test_iface1, IDCU_MOD_PRIO_NORMAL);
    idcu_module_registry_register(&reg, &g_test_iface2, IDCU_MOD_PRIO_NORMAL);
    
    idcu_kernel_init(&k);
    
    k.sb_cnt = 2;
    k.tracked_cnt = 2;
    k.tracked_modules[0].iface = &g_test_iface1;
    k.tracked_modules[0].state = IDCU_MOD_STATE_RUNNING;
    k.tracked_modules[1].iface = &g_test_iface2;
    k.tracked_modules[1].state = IDCU_MOD_STATE_RUNNING;
    
    idcu_kernel_stop(&k);
    
    idcu_module_registry_destroy(&reg);
    
    IDCU_TEST_PASS();
}

static void test_kernel_should_exit_flag(void) {
    idcu_MicroKernel k;
    
    idcu_kernel_init(&k);
    
    IDCU_TEST_ASSERT(k.should_exit == 0, "should_exit should be 0 initially");
    
    k.should_exit = 1;
    IDCU_TEST_ASSERT(k.should_exit == 1, "should_exit should be settable to 1");
    
    IDCU_TEST_PASS();
}

static void test_kernel_tracked_modules(void) {
    idcu_MicroKernel k;
    
    idcu_kernel_init(&k);
    
    k.tracked_cnt = 2;
    k.tracked_modules[0].iface = &g_test_iface1;
    k.tracked_modules[0].state = IDCU_MOD_STATE_RUNNING;
    k.tracked_modules[1].iface = &g_test_iface2;
    k.tracked_modules[1].state = IDCU_MOD_STATE_RUNNING;
    
    IDCU_TEST_ASSERT(k.tracked_cnt == 2, "tracked_cnt should be 2");
    IDCU_TEST_ASSERT(k.tracked_modules[0].iface != NULL, "Module 0 interface should not be NULL");
    IDCU_TEST_ASSERT(k.tracked_modules[1].iface != NULL, "Module 1 interface should not be NULL");
    IDCU_TEST_ASSERT(k.tracked_modules[0].state == IDCU_MOD_STATE_RUNNING, "Module 0 should be running");
    IDCU_TEST_ASSERT(k.tracked_modules[1].state == IDCU_MOD_STATE_RUNNING, "Module 1 should be running");
    
    IDCU_TEST_PASS();
}

static void test_kernel_sandbox_init(void) {
    idcu_MicroKernel k;
    
    idcu_kernel_init(&k);
    
    k.sb_cnt = 2;
    k.sandbox[0].module_id = 0;
    k.sandbox[0].perm = IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN;
    k.sandbox[1].module_id = 1;
    k.sandbox[1].perm = IDCU_PERM_SEND | IDCU_PERM_RECV;
    
    IDCU_TEST_ASSERT(k.sb_cnt == 2, "sb_cnt should be 2");
    IDCU_TEST_ASSERT(k.sandbox[0].module_id == 0, "Sandbox 0 module_id should be 0");
    IDCU_TEST_ASSERT(k.sandbox[1].module_id == 1, "Sandbox 1 module_id should be 1");
    
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Micro Kernel Tests");
    
    idcu_test_suite_add_test(&g_suite, "kernel_init", test_kernel_init);
    idcu_test_suite_add_test(&g_suite, "kernel_stop", test_kernel_stop);
    idcu_test_suite_add_test(&g_suite, "kernel_set_signal_handler", test_kernel_set_signal_handler);
    idcu_test_suite_add_test(&g_suite, "kernel_start_stop_lifecycle", test_kernel_start_stop_lifecycle);
    idcu_test_suite_add_test(&g_suite, "kernel_should_exit_flag", test_kernel_should_exit_flag);
    idcu_test_suite_add_test(&g_suite, "kernel_tracked_modules", test_kernel_tracked_modules);
    idcu_test_suite_add_test(&g_suite, "kernel_sandbox_init", test_kernel_sandbox_init);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
