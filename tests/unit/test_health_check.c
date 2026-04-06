#include "test/test_framework.h"
#include "health_check.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_health_monitor_init_destroy(void) {
    idcu_HealthMonitor monitor;
    int ret = idcu_health_monitor_init(&monitor);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_health_monitor_init should succeed");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_register_unregister(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    int ret = idcu_health_register_module(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Register module 1 should succeed");
    
    ret = idcu_health_register_module(&monitor, 2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Register module 2 should succeed");
    
    ret = idcu_health_unregister_module(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Unregister module 1 should succeed");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_heartbeat(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    
    int ret = idcu_health_update_heartbeat(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Update heartbeat should succeed");
    
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
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Report error should succeed");
    
    const idcu_ModuleHealth* info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info != NULL, "Get info should succeed");
    IDCU_TEST_ASSERT(info->error_count == 1, "Error count should be 1");
    
    ret = idcu_health_report_error(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Report error 2 should succeed");
    info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info->error_count == 2, "Error count should be 2");
    
    ret = idcu_health_report_restart(&monitor, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Report restart should succeed");
    info = idcu_health_get_info(&monitor, 1);
    IDCU_TEST_ASSERT(info->restart_count == 1, "Restart count should be 1");
    IDCU_TEST_ASSERT(info->error_count == 0, "Error count should reset to 0");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_status_to_string(void) {
    const char* str;
    
    str = idcu_health_status_to_string(IDCU_HEALTH_HEALTHY);
    IDCU_TEST_ASSERT(strcmp(str, "HEALTHY") == 0, "HEALTHY should be correct");
    
    str = idcu_health_status_to_string(IDCU_HEALTH_WARNING);
    IDCU_TEST_ASSERT(strcmp(str, "WARNING") == 0, "WARNING should be correct");
    
    str = idcu_health_status_to_string(IDCU_HEALTH_CRITICAL);
    IDCU_TEST_ASSERT(strcmp(str, "CRITICAL") == 0, "CRITICAL should be correct");
    
    str = idcu_health_status_to_string(IDCU_HEALTH_DEAD);
    IDCU_TEST_ASSERT(strcmp(str, "DEAD") == 0, "DEAD should be correct");
    
    str = idcu_health_status_to_string(IDCU_HEALTH_UNKNOWN);
    IDCU_TEST_ASSERT(strcmp(str, "UNKNOWN") == 0, "UNKNOWN should be correct");
    
    IDCU_TEST_PASS();
}

static void test_health_summary(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    idcu_health_register_module(&monitor, 2);
    idcu_health_register_module(&monitor, 3);
    
    idcu_HealthSummary summary;
    idcu_health_get_summary(&monitor, &summary);
    
    IDCU_TEST_ASSERT(summary.total_count == 3, "Total count should be 3");
    IDCU_TEST_ASSERT(summary.healthy_count == 3, "Healthy count should be 3");
    
    uint32_t count = idcu_health_get_module_count(&monitor);
    IDCU_TEST_ASSERT(count == 3, "Module count should be 3");
    
    idcu_health_monitor_destroy(&monitor);
    IDCU_TEST_PASS();
}

static void test_health_overall_status(void) {
    idcu_HealthMonitor monitor;
    idcu_health_monitor_init(&monitor);
    
    idcu_health_register_module(&monitor, 1);
    idcu_health_register_module(&monitor, 2);
    
    idcu_HealthStatus overall = idcu_health_get_overall_status(&monitor);
    IDCU_TEST_ASSERT(overall == IDCU_HEALTH_HEALTHY, "Overall status should be HEALTHY");
    
    idcu_health_set_thresholds(&monitor, 10000, 2, 5);
    
    idcu_health_report_error(&monitor, 1);
    idcu_health_report_error(&monitor, 1);
    idcu_health_report_error(&monitor, 1);
    
    overall = idcu_health_get_overall_status(&monitor);
    IDCU_TEST_ASSERT(overall == IDCU_HEALTH_WARNING, "Overall status should be WARNING after errors");
    
    idcu_health_reset_error_count(&monitor, 1);
    overall = idcu_health_get_overall_status(&monitor);
    IDCU_TEST_ASSERT(overall == IDCU_HEALTH_HEALTHY, "Overall status should be HEALTHY after reset");
    
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
    idcu_test_suite_add_test(&g_suite, "health_status_to_string", test_health_status_to_string);
    idcu_test_suite_add_test(&g_suite, "health_summary", test_health_summary);
    idcu_test_suite_add_test(&g_suite, "health_overall_status", test_health_overall_status);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
