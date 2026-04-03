#include "test/test_framework.h"
#include "security/sandbox.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_sandbox_init_destroy(void) {
    idcu_Sandbox sb;
    
    int ret = idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_RECV);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_init should succeed");
    IDCU_TEST_ASSERT(sb.module_id == 1, "Module ID should be 1");
    IDCU_TEST_ASSERT(sb.perm == (IDCU_PERM_SEND | IDCU_PERM_RECV), "Permissions should be SEND|RECV");
    IDCU_TEST_ASSERT(sb.quota == 0, "Quota should be 0 initially");
    
    idcu_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_sandbox_init_null_param(void) {
    int ret = idcu_sandbox_init(NULL, 1, 0);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "idcu_sandbox_init with NULL should fail");
    IDCU_TEST_PASS();
}

static void test_sandbox_perm_check(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_RECV);
    
    int ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have PERM_SEND permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RECV);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have PERM_RECV permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Should not have PERM_RUN permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND | IDCU_PERM_RECV);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have both SEND and RECV permissions");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND | IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Should not have both SEND and RUN permissions");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_perm_check_null_param(void) {
    int ret = idcu_sandbox_perm_check(NULL, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "idcu_sandbox_perm_check with NULL should fail");
    IDCU_TEST_PASS();
}

static void test_sandbox_set_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_sandbox_set_perm(&sb, IDCU_PERM_RUN | IDCU_PERM_HW);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_set_perm should succeed");
    IDCU_TEST_ASSERT(sb.perm == (IDCU_PERM_RUN | IDCU_PERM_HW), "Permissions should be RUN|HW");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have PERM_RUN permission");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_set_perm_null_param(void) {
    int ret = idcu_sandbox_set_perm(NULL, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "idcu_sandbox_set_perm with NULL should fail");
    IDCU_TEST_PASS();
}

static void test_sandbox_add_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND);
    
    int ret = idcu_sandbox_add_perm(&sb, IDCU_PERM_RECV);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_add_perm should succeed");
    IDCU_TEST_ASSERT(sb.perm == (IDCU_PERM_SEND | IDCU_PERM_RECV), "Permissions should be SEND|RECV");
    
    ret = idcu_sandbox_add_perm(&sb, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_add_perm should succeed again");
    IDCU_TEST_ASSERT(sb.perm == (IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN), "Permissions should be SEND|RECV|RUN");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_add_perm_null_param(void) {
    int ret = idcu_sandbox_add_perm(NULL, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "idcu_sandbox_add_perm with NULL should fail");
    IDCU_TEST_PASS();
}

static void test_sandbox_remove_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_RECV | IDCU_PERM_RUN);
    
    int ret = idcu_sandbox_remove_perm(&sb, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_remove_perm should succeed");
    IDCU_TEST_ASSERT(sb.perm == (IDCU_PERM_SEND | IDCU_PERM_RECV), "Permissions should be SEND|RECV");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Should not have PERM_RUN permission after removal");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_remove_perm_null_param(void) {
    int ret = idcu_sandbox_remove_perm(NULL, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "idcu_sandbox_remove_perm with NULL should fail");
    IDCU_TEST_PASS();
}

static void test_sandbox_get_perm(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_FILE);
    
    uint32_t perm = idcu_sandbox_get_perm(&sb);
    IDCU_TEST_ASSERT(perm == (IDCU_PERM_SEND | IDCU_PERM_FILE), "get_perm should return correct permissions");
    
    perm = idcu_sandbox_get_perm(NULL);
    IDCU_TEST_ASSERT(perm == 0, "get_perm with NULL should return 0");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_quota(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_sandbox_set_quota(&sb, 1000);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_set_quota should succeed");
    IDCU_TEST_ASSERT(sb.quota == 1000, "Quota should be 1000");
    
    uint32_t quota = idcu_sandbox_get_quota(&sb);
    IDCU_TEST_ASSERT(quota == 1000, "get_quota should return 1000");
    
    quota = idcu_sandbox_get_quota(NULL);
    IDCU_TEST_ASSERT(quota == 0, "get_quota with NULL should return 0");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_set_quota_null_param(void) {
    int ret = idcu_sandbox_set_quota(NULL, 100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "idcu_sandbox_set_quota with NULL should fail");
    IDCU_TEST_PASS();
}

static void test_sandbox_all_permissions(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, IDCU_PERM_ALL);
    
    int ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have SEND permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RECV);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have RECV permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have RUN permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_HW);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have HW permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_FILE);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have FILE permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_NETWORK);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Should have NETWORK permission");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_init_destroy(void) {
    int ret = idcu_sandbox_registry_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_registry_init should succeed");
    
    ret = idcu_sandbox_registry_init();
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_registry_init should succeed when already initialized");
    
    idcu_sandbox_registry_destroy();
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_add_remove(void) {
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 100, IDCU_PERM_SEND);
    idcu_sandbox_init(&sb2, 200, IDCU_PERM_RECV);
    
    idcu_sandbox_registry_init();
    
    int ret = idcu_sandbox_registry_add(&sb1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add sandbox 1 should succeed");
    
    ret = idcu_sandbox_registry_add(&sb2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add sandbox 2 should succeed");
    
    ret = idcu_sandbox_registry_add(&sb1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_ALREADY_EXISTS, "Add duplicate sandbox should fail");
    
    ret = idcu_sandbox_registry_remove(100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove sandbox 1 should succeed");
    
    ret = idcu_sandbox_registry_remove(100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "Remove non-existent sandbox should fail");
    
    idcu_sandbox_registry_destroy();
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_add_null_param(void) {
    idcu_sandbox_registry_init();
    
    int ret = idcu_sandbox_registry_add(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "Add NULL sandbox should fail");
    
    idcu_sandbox_registry_destroy();
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_add_without_init(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_sandbox_registry_add(&sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_INITIALIZED, "Add sandbox without init should fail");
    
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_get(void) {
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 100, IDCU_PERM_SEND);
    idcu_sandbox_init(&sb2, 200, IDCU_PERM_RECV);
    
    idcu_sandbox_registry_init();
    idcu_sandbox_registry_add(&sb1);
    idcu_sandbox_registry_add(&sb2);
    
    idcu_Sandbox* sb = idcu_sandbox_registry_get(100);
    IDCU_TEST_ASSERT(sb != NULL, "Get sandbox 100 should not return NULL");
    IDCU_TEST_ASSERT(sb->module_id == 100, "Module ID should be 100");
    
    sb = idcu_sandbox_registry_get(200);
    IDCU_TEST_ASSERT(sb != NULL, "Get sandbox 200 should not return NULL");
    IDCU_TEST_ASSERT(sb->module_id == 200, "Module ID should be 200");
    
    sb = idcu_sandbox_registry_get(999);
    IDCU_TEST_ASSERT(sb == NULL, "Get non-existent sandbox should return NULL");
    
    idcu_sandbox_registry_destroy();
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_get_without_init(void) {
    idcu_Sandbox* sb = idcu_sandbox_registry_get(1);
    IDCU_TEST_ASSERT(sb == NULL, "Get sandbox without init should return NULL");
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_check_perm(void) {
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 100, IDCU_PERM_SEND | IDCU_PERM_RECV);
    idcu_sandbox_init(&sb2, 200, IDCU_PERM_RUN);
    
    idcu_sandbox_registry_init();
    idcu_sandbox_registry_add(&sb1);
    idcu_sandbox_registry_add(&sb2);
    
    int ret = idcu_sandbox_registry_check_perm(100, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check perm SEND for module 100 should succeed");
    
    ret = idcu_sandbox_registry_check_perm(100, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Check perm RUN for module 100 should fail");
    
    ret = idcu_sandbox_registry_check_perm(200, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check perm RUN for module 200 should succeed");
    
    ret = idcu_sandbox_registry_check_perm(999, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "Check perm for non-existent module should fail");
    
    idcu_sandbox_registry_destroy();
    IDCU_TEST_PASS();
}

static void test_sandbox_registry_full(void) {
    idcu_Sandbox sandboxes[IDCU_MAX_SANDBOXES];
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        idcu_sandbox_init(&sandboxes[i], i + 1, 0);
    }
    
    idcu_sandbox_registry_init();
    
    int all_succeeded = 1;
    for (int i = 0; i < IDCU_MAX_SANDBOXES; i++) {
        int ret = idcu_sandbox_registry_add(&sandboxes[i]);
        if (ret != IDCU_ERR_OK) {
            all_succeeded = 0;
            break;
        }
    }
    IDCU_TEST_ASSERT(all_succeeded, "All sandboxes should be added successfully");
    
    idcu_Sandbox extra_sb;
    idcu_sandbox_init(&extra_sb, IDCU_MAX_SANDBOXES + 1, 0);
    int ret = idcu_sandbox_registry_add(&extra_sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NO_MEMORY, "Add extra sandbox should fail with NO_MEMORY");
    
    idcu_sandbox_registry_destroy();
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Sandbox Tests");
    
    idcu_test_suite_add_test(&g_suite, "sandbox_init_destroy", test_sandbox_init_destroy);
    idcu_test_suite_add_test(&g_suite, "sandbox_init_null_param", test_sandbox_init_null_param);
    idcu_test_suite_add_test(&g_suite, "sandbox_perm_check", test_sandbox_perm_check);
    idcu_test_suite_add_test(&g_suite, "sandbox_perm_check_null_param", test_sandbox_perm_check_null_param);
    idcu_test_suite_add_test(&g_suite, "sandbox_set_perm", test_sandbox_set_perm);
    idcu_test_suite_add_test(&g_suite, "sandbox_set_perm_null_param", test_sandbox_set_perm_null_param);
    idcu_test_suite_add_test(&g_suite, "sandbox_add_perm", test_sandbox_add_perm);
    idcu_test_suite_add_test(&g_suite, "sandbox_add_perm_null_param", test_sandbox_add_perm_null_param);
    idcu_test_suite_add_test(&g_suite, "sandbox_remove_perm", test_sandbox_remove_perm);
    idcu_test_suite_add_test(&g_suite, "sandbox_remove_perm_null_param", test_sandbox_remove_perm_null_param);
    idcu_test_suite_add_test(&g_suite, "sandbox_get_perm", test_sandbox_get_perm);
    idcu_test_suite_add_test(&g_suite, "sandbox_quota", test_sandbox_quota);
    idcu_test_suite_add_test(&g_suite, "sandbox_set_quota_null_param", test_sandbox_set_quota_null_param);
    idcu_test_suite_add_test(&g_suite, "sandbox_all_permissions", test_sandbox_all_permissions);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_init_destroy", test_sandbox_registry_init_destroy);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_add_remove", test_sandbox_registry_add_remove);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_add_null_param", test_sandbox_registry_add_null_param);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_add_without_init", test_sandbox_registry_add_without_init);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_get", test_sandbox_registry_get);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_get_without_init", test_sandbox_registry_get_without_init);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_check_perm", test_sandbox_registry_check_perm);
    idcu_test_suite_add_test(&g_suite, "sandbox_registry_full", test_sandbox_registry_full);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
