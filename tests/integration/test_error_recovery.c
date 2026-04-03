#include "test_framework.h"
#include "health_check.h"
#include "dynamic_module.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_health_monitor_init_destroy(void) {
    HealthMonitor monitor;
    int ret = health_monitor_init(&monitor);
    TEST_ASSERT(ret == ERR_OK, "health_monitor_init should succeed");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_health_module_registration(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    int ret = health_register_module(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "health_register_module should succeed");
    
    ret = health_register_module(&monitor, 2);
    TEST_ASSERT(ret == ERR_OK, "health_register_module should succeed for module 2");
    
    const ModuleHealth* info = health_get_info(&monitor, 1);
    TEST_ASSERT(info != NULL, "health_get_info should return info for module 1");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_heartbeat_update(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    
    int ret = health_update_heartbeat(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "health_update_heartbeat should succeed");
    
    HealthStatus status = health_get_status(&monitor, 1);
    TEST_ASSERT(status == HEALTH_STATUS_HEALTHY, "Status should be HEALTHY after heartbeat");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_error_reporting(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    
    for (int i = 0; i < 5; i++) {
        health_report_error(&monitor, 1);
    }
    
    const ModuleHealth* info = health_get_info(&monitor, 1);
    TEST_ASSERT(info != NULL, "health_get_info should return info");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_restart_count(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    
    int ret = health_report_restart(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "health_report_restart should succeed");
    
    ret = health_report_restart(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "health_report_restart should succeed again");
    
    const ModuleHealth* info = health_get_info(&monitor, 1);
    TEST_ASSERT(info != NULL, "health_get_info should return info");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_batch_health_check(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    health_register_module(&monitor, 2);
    health_register_module(&monitor, 3);
    
    health_update_heartbeat(&monitor, 1);
    health_update_heartbeat(&monitor, 2);
    
    int ret = health_check_all(&monitor);
    TEST_ASSERT(ret == ERR_OK, "health_check_all should succeed");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_module_unload_recovery(void) {
    DynamicLoader loader;
    int ret = dynamic_loader_init(&loader, NULL);
    TEST_ASSERT(ret == ERR_OK, "dynamic_loader_init should succeed");
    
    DynamicModule* mod = dynamic_loader_find_module(&loader, "nonexistent");
    TEST_ASSERT(mod == NULL, "Find should return NULL for nonexistent module");
    
    dynamic_loader_destroy(&loader);
    TEST_PASS();
}

static void test_health_callback(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_module_unregistration(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    health_register_module(&monitor, 2);
    
    int ret = health_unregister_module(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "health_unregister_module should succeed");
    
    const ModuleHealth* info = health_get_info(&monitor, 1);
    TEST_ASSERT(info == NULL, "health_get_info should return NULL after unregister");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Error Recovery Integration Tests");
    
    test_suite_add_test(&g_suite, "health_monitor_init_destroy", test_health_monitor_init_destroy);
    test_suite_add_test(&g_suite, "health_module_registration", test_health_module_registration);
    test_suite_add_test(&g_suite, "heartbeat_update", test_heartbeat_update);
    test_suite_add_test(&g_suite, "error_reporting", test_error_reporting);
    test_suite_add_test(&g_suite, "restart_count", test_restart_count);
    test_suite_add_test(&g_suite, "batch_health_check", test_batch_health_check);
    test_suite_add_test(&g_suite, "module_unload_recovery", test_module_unload_recovery);
    test_suite_add_test(&g_suite, "health_callback", test_health_callback);
    test_suite_add_test(&g_suite, "module_unregistration", test_module_unregistration);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
