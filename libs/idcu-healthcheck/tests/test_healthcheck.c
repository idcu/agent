#include <idcu/testframework/testframework.h>
#include <idcu/healthcheck/healthcheck.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int custom_check_success(void* user_data, idcu_HealthCheckResult* result) {
    (void)user_data;
    idcu_healthcheck_result_init(result);
    idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_PASS);
    idcu_healthcheck_result_set_message(result, "OK");
    return IDCU_SUCCESS;
}

static int custom_check_warn(void* user_data, idcu_HealthCheckResult* result) {
    (void)user_data;
    idcu_healthcheck_result_init(result);
    idcu_healthcheck_result_set_status(result, IDCU_HEALTH_STATUS_WARN);
    idcu_healthcheck_result_set_message(result, "Warning");
    return IDCU_SUCCESS;
}

IDCU_TEST_CASE(healthchecker, init_destroy) {
    idcu_HealthChecker checker;
    int ret = idcu_healthchecker_init(&checker);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthchecker_destroy(&checker);
}

IDCU_TEST_CASE(healthchecker, healthcheck_init_destroy) {
    idcu_HealthCheck check;
    int ret = idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, result_init_destroy) {
    idcu_HealthCheckResult result;
    int ret = idcu_healthcheck_result_init(&result);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_result_destroy(&result);
}

IDCU_TEST_CASE(healthchecker, set_custom) {
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    
    int ret = idcu_healthcheck_set_custom(&check, custom_check_success, NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, set_http) {
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "http_check", IDCU_HEALTH_CHECK_TYPE_HTTP);
    
    int ret = idcu_healthcheck_set_http(&check, "http://example.com", 5000, 200);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, set_disk) {
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "disk_check", IDCU_HEALTH_CHECK_TYPE_DISK);
    
    int ret = idcu_healthcheck_set_disk(&check, "/", 1024 * 1024 * 1024, 10.0);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, set_memory) {
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "memory_check", IDCU_HEALTH_CHECK_TYPE_MEMORY);
    
    int ret = idcu_healthcheck_set_memory(&check, 1024 * 1024 * 1024, 10.0);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, set_tcp) {
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "tcp_check", IDCU_HEALTH_CHECK_TYPE_TCP);
    
    int ret = idcu_healthcheck_set_tcp(&check, "example.com", 80, 5000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, set_interval) {
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    
    int ret = idcu_healthcheck_set_interval(&check, 60000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, set_timeout) {
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    
    int ret = idcu_healthcheck_set_timeout(&check, 5000);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
}

IDCU_TEST_CASE(healthchecker, add_check) {
    idcu_HealthChecker checker;
    idcu_healthchecker_init(&checker);
    
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    idcu_healthcheck_set_custom(&check, custom_check_success, NULL);
    
    int ret = idcu_healthchecker_add_check(&checker, &check);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
    idcu_healthchecker_destroy(&checker);
}

IDCU_TEST_CASE(healthchecker, remove_check) {
    idcu_HealthChecker checker;
    idcu_healthchecker_init(&checker);
    
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    idcu_healthcheck_set_custom(&check, custom_check_success, NULL);
    
    idcu_healthchecker_add_check(&checker, &check);
    
    int ret = idcu_healthchecker_remove_check(&checker, "test_check");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_destroy(&check);
    idcu_healthchecker_destroy(&checker);
}

IDCU_TEST_CASE(healthchecker, check_all) {
    idcu_HealthChecker checker;
    idcu_healthchecker_init(&checker);
    
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    idcu_healthcheck_set_custom(&check, custom_check_success, NULL);
    
    idcu_healthchecker_add_check(&checker, &check);
    
    int ret = idcu_healthchecker_check_all(&checker);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_HealthStatus overall = idcu_healthchecker_get_overall_status(&checker);
    IDCU_TEST_ASSERT_EQUAL(IDCU_HEALTH_STATUS_PASS, overall);
    
    idcu_healthcheck_destroy(&check);
    idcu_healthchecker_destroy(&checker);
}

IDCU_TEST_CASE(healthchecker, to_json) {
    idcu_HealthChecker checker;
    idcu_healthchecker_init(&checker);
    
    idcu_HealthCheck check;
    idcu_healthcheck_init(&check, "test_check", IDCU_HEALTH_CHECK_TYPE_CUSTOM);
    idcu_healthcheck_set_custom(&check, custom_check_success, NULL);
    
    idcu_healthchecker_add_check(&checker, &check);
    idcu_healthchecker_check_all(&checker);
    
    char buffer[2048];
    int ret = idcu_healthchecker_to_json(&checker, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(ret > 0);
    IDCU_TEST_ASSERT(strstr(buffer, "test_check") != NULL);
    
    idcu_healthcheck_destroy(&check);
    idcu_healthchecker_destroy(&checker);
}

IDCU_TEST_CASE(healthchecker, health_status_to_string) {
    const char* str = idcu_health_status_to_string(IDCU_HEALTH_STATUS_PASS);
    IDCU_TEST_ASSERT_STRING_EQUAL("pass", str);
    
    str = idcu_health_status_to_string(IDCU_HEALTH_STATUS_FAIL);
    IDCU_TEST_ASSERT_STRING_EQUAL("fail", str);
}

IDCU_TEST_CASE(healthchecker, result_set_status) {
    idcu_HealthCheckResult result;
    idcu_healthcheck_result_init(&result);
    
    int ret = idcu_healthcheck_result_set_status(&result, IDCU_HEALTH_STATUS_PASS);
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_result_destroy(&result);
}

IDCU_TEST_CASE(healthchecker, result_set_message) {
    idcu_HealthCheckResult result;
    idcu_healthcheck_result_init(&result);
    
    int ret = idcu_healthcheck_result_set_message(&result, "test message");
    IDCU_TEST_ASSERT_EQUAL(IDCU_SUCCESS, ret);
    
    idcu_healthcheck_result_destroy(&result);
}

int main(void) {
    return idcu_test_run_all();
}
