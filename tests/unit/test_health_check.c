#include "test/test_framework.h"
#include "monitor/health_check.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_health_monitor_init_destroy(void) {
    idcu_HealthMonitor monitor;
    int ret = idcu_health_monitor_init(&monitor);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_health_monitor_init should succeed");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_register_unregister(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    int ret = idcu_health_register_module(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Register module 1 should succeed");
    
    ret = idcu_health_register_module(&monitor, 2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Register module 2 should succeed");
    
    ret = idcu_health_unregister_module(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Unregister module 1 should succeed");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_heartbeat(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    
    int ret = idcu_health_update_heartbeat(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Update heartbeat should succeed");
    
    idcu_HealthStatus status = idcu_health_get_status(&monitor, 1);
    IDCU_TEST_ASSERT(status == IDCU_HEALTH_HEALTHY, "Status should be HEALTHY");
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info != NULL, "Get info should succeed");
    IDCU_TEST_ASSERT(info->module_id == 1, "Module ID should be 1");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_error_reporting(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    
    int ret = idcu_health_report_error(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Report error should succeed");
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, 1);
    
    ret = idcu_health_report_error(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Report error 2 should succeed");
    info = idcu_health_get_info(&monitor, 1);
    
    ret = idcu_health_report_restart(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Report restart should succeed");
    info = idcu_health_get_info(&monitor, 1);
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Health Check Tests");
    
    idcu_test_suite_add_test(&g_suite, "health_monitor_init_destroy", test_health_monitor_init_destroy);
    idcu_test_suite_add_test(&g_suite, "health_register_unregister", test_health_register_unregister);
    idcu_test_suite_add_test(&g_suite, "health_heartbeat", test_health_heartbeat);
    idcu_test_suite_add_test(&g_suite, "health_error_reporting", test_health_error_reporting);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
