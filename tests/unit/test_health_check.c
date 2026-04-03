#include "test/test_framework.h"
#include "monitor/health_check.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_health_monitor_init_destroy(void) {
    HealthMonitor monitor;
    int ret = health_monitor_init(&monitor);
    TEST_ASSERT(ret == ERR_OK, "health_monitor_init should succeed");
    TEST_ASSERT(monitor.count == 0, "Module count should be 0");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_health_register_unregister(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    int ret = health_register_module(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "Register module 1 should succeed");
    TEST_ASSERT(monitor.count == 1, "Module count should be 1");
    
    ret = health_register_module(&monitor, 2);
    TEST_ASSERT(ret == ERR_OK, "Register module 2 should succeed");
    TEST_ASSERT(monitor.count == 2, "Module count should be 2");
    
    ret = health_unregister_module(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "Unregister module 1 should succeed");
    TEST_ASSERT(monitor.count == 1, "Module count should be 1");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_health_heartbeat(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    
    int ret = health_update_heartbeat(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "Update heartbeat should succeed");
    
    HealthStatus status = health_get_status(&monitor, 1);
    TEST_ASSERT(status == HEALTH_HEALTHY, "Status should be HEALTHY");
    
    const ModuleHealth* info = health_get_info(&monitor, 1);
    TEST_ASSERT(info != NULL, "Get info should succeed");
    TEST_ASSERT(info->module_id == 1, "Module ID should be 1");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

static void test_health_error_reporting(void) {
    HealthMonitor monitor;
    health_monitor_init(&monitor);
    
    health_register_module(&monitor, 1);
    
    int ret = health_report_error(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "Report error should succeed");
    
    const ModuleHealth* info = health_get_info(&monitor, 1);
    TEST_ASSERT(info->error_count == 1, "Error count should be 1");
    
    ret = health_report_error(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "Report error 2 should succeed");
    info = health_get_info(&monitor, 1);
    TEST_ASSERT(info->error_count == 2, "Error count should be 2");
    
    ret = health_report_restart(&monitor, 1);
    TEST_ASSERT(ret == ERR_OK, "Report restart should succeed");
    info = health_get_info(&monitor, 1);
    TEST_ASSERT(info->restart_count == 1, "Restart count should be 1");
    
    health_monitor_destroy(&monitor);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Health Check Tests");
    
    test_suite_add_test(&g_suite, "health_monitor_init_destroy", test_health_monitor_init_destroy);
    test_suite_add_test(&g_suite, "health_register_unregister", test_health_register_unregister);
    test_suite_add_test(&g_suite, "health_heartbeat", test_health_heartbeat);
    test_suite_add_test(&g_suite, "health_error_reporting", test_health_error_reporting);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
