#include "test/test_framework.h"
#include "monitor/health_check.h"
#include "module/module_registry.h"
#include "module/module_def.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static int g_should_fail_init = 0;
static int g_should_fail_run = 0;
static int g_init_called = 0;
static int g_run_called = 0;
static int g_stop_called = 0;

static int flaky_module_init(void) {
    g_init_called++;
    if (g_should_fail_init) {
        return IDCU_ERR_GENERAL;
    }
    return IDCU_ERR_OK;
}

static int flaky_module_run(void) {
    g_run_called++;
    if (g_should_fail_run) {
        return IDCU_ERR_GENERAL;
    }
    return IDCU_ERR_OK;
}

static int flaky_module_stop(void) {
    g_stop_called++;
    return IDCU_ERR_OK;
}

static const idcu_ModuleInterface g_flaky_module = {
    .name = "flaky_module",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = flaky_module_init,
    .run = flaky_module_run,
    .stop = flaky_module_stop
};

static void test_health_monitor_init_destroy(void) {
    idcu_HealthMonitor monitor;
    int ret = idcu_health_monitor_init(&monitor);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_monitor_init should succeed");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_module_registration(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    int ret = idcu_health_register_module(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_register_module should succeed");
    
    ret = idcu_health_register_module(&monitor, 2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_register_module should succeed for module 2");
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info != NULL, "health_get_info should return info for module 1");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_heartbeat_update(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    
    int ret = idcu_health_update_heartbeat(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_update_heartbeat should succeed");
    
    idcu_HealthStatus status = idcu_health_get_status(&monitor, 1);
    IDCU_TEST_ASSERT(status == IDCU_HEALTH_HEALTHY, "Status should be HEALTHY after heartbeat");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_error_reporting(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    
    for (int i = 0; i < 5; i++) {
        idcu_health_report_error(&monitor, 1);
    }
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info != NULL, "health_get_info should return info");
    IDCU_TEST_ASSERT(info->error_count == 5, "Error count should be 5");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_restart_count(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    
    int ret = idcu_health_report_restart(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_report_restart should succeed");
    
    ret = idcu_health_report_restart(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_report_restart should succeed again");
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info != NULL, "health_get_info should return info");
    IDCU_TEST_ASSERT(info->restart_count == 2, "Restart count should be 2");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_batch_health_check(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    idcu_health_register_module(&monitor, 2);
    idcu_health_register_module(&monitor, 3);
    
    idcu_health_update_heartbeat(&monitor, 1);
    idcu_health_update_heartbeat(&monitor, 2);
    
    int ret = idcu_health_check_all(&monitor);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_check_all should succeed");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_module_error_recovery(void) {
    idcu_ModuleRegistry reg;
    idcu_HealthMonitor monitor;
    
    idcu_module_registry_init(&reg);
    idcu_health_monitor_init(&monitor);
    
    g_should_fail_init = 0;
    g_should_fail_run = 0;
    g_init_called = 0;
    g_run_called = 0;
    g_stop_called = 0;
    
    idcu_module_registry_register(&reg, &g_flaky_module, IDCU_MOD_PRIO_NORMAL);
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(&reg, "flaky_module");
    IDCU_TEST_ASSERT(mod != NULL, "Module should be found");
    
    idcu_health_register_module(&monitor, mod->module_id);
    
    int ret = idcu_module_registry_init_module(&reg, mod->module_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init should succeed");
    IDCU_TEST_ASSERT(g_init_called == 1, "Init should be called once");
    
    ret = idcu_module_registry_run_module(&reg, mod->module_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Run should succeed");
    IDCU_TEST_ASSERT(g_run_called == 1, "Run should be called once");
    
    g_should_fail_run = 1;
    ret = idcu_module_registry_run_module(&reg, mod->module_id);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK, "Run should fail when should_fail_run is set");
    
    idcu_health_report_error(&monitor, mod->module_id);
    
    g_should_fail_run = 0;
    idcu_module_registry_stop_module(&reg, mod->module_id);
    IDCU_TEST_ASSERT(g_stop_called == 1, "Stop should be called once");
    
    idcu_health_report_restart(&monitor, mod->module_id);
    
    ret = idcu_module_registry_init_module(&reg, mod->module_id);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Re-init should succeed");
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, mod->module_id);
    IDCU_TEST_ASSERT(info != NULL, "Health info should exist");
    IDCU_TEST_ASSERT(info->error_count == 1, "Should have 1 error reported");
    IDCU_TEST_ASSERT(info->restart_count == 1, "Should have 1 restart reported");
    
    idcu_health_monitor_destroy(&monitor);
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_health_callback(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_module_unregistration(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    idcu_health_register_module(&monitor, 2);
    
    int ret = idcu_health_unregister_module(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "health_unregister_module should succeed");
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info == NULL, "health_get_info should return NULL after unregister");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_module_recovery_flow(void) {
    idcu_ModuleRegistry reg;
    idcu_HealthMonitor monitor;
    
    idcu_module_registry_init(&reg);
    idcu_health_monitor_init(&monitor);
    
    g_should_fail_init = 0;
    g_should_fail_run = 0;
    g_init_called = 0;
    g_run_called = 0;
    g_stop_called = 0;
    
    idcu_module_registry_register(&reg, &g_flaky_module, IDCU_MOD_PRIO_NORMAL);
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(&reg, "flaky_module");
    IDCU_TEST_ASSERT(mod != NULL, "Module should be found");
    
    idcu_health_register_module(&monitor, mod->module_id);
    
    int ret = idcu_module_registry_init_all(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Init all should succeed");
    
    ret = idcu_module_registry_run_all(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Run all should succeed");
    
    idcu_module_registry_stop_all(&reg);
    
    idcu_health_monitor_destroy(&monitor);
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Error Recovery Integration Tests");
    
    idcu_test_suite_add_test(&g_suite, "health_monitor_init_destroy", test_health_monitor_init_destroy);
    idcu_test_suite_add_test(&g_suite, "health_module_registration", test_health_module_registration);
    idcu_test_suite_add_test(&g_suite, "heartbeat_update", test_heartbeat_update);
    idcu_test_suite_add_test(&g_suite, "error_reporting", test_error_reporting);
    idcu_test_suite_add_test(&g_suite, "restart_count", test_restart_count);
    idcu_test_suite_add_test(&g_suite, "batch_health_check", test_batch_health_check);
    idcu_test_suite_add_test(&g_suite, "module_error_recovery", test_module_error_recovery);
    idcu_test_suite_add_test(&g_suite, "health_callback", test_health_callback);
    idcu_test_suite_add_test(&g_suite, "module_unregistration", test_module_unregistration);
    idcu_test_suite_add_test(&g_suite, "module_recovery_flow", test_module_recovery_flow);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
