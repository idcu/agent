#include "test/test_framework.h"
#include "security/sandbox.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_sandbox_init_destroy(void) {
    Sandbox sb;
    int ret = sandbox_init(&sb, 1, PERM_SEND | PERM_RECV);
    TEST_ASSERT(ret == ERR_OK, "sandbox_init should succeed");
    TEST_ASSERT(sb.module_id == 1, "Module ID should be 1");
    TEST_ASSERT(sb.active != 0, "Sandbox should be active");
    
    sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_sandbox_perm_check(void) {
    Sandbox sb;
    sandbox_init(&sb, 1, PERM_SEND | PERM_RECV);
    
    int ret = sandbox_perm_check(&sb, PERM_SEND);
    TEST_ASSERT(ret == 0, "Should have PERM_SEND permission");
    
    ret = sandbox_perm_check(&sb, PERM_RECV);
    TEST_ASSERT(ret == 0, "Should have PERM_RECV permission");
    
    ret = sandbox_perm_check(&sb, PERM_RUN);
    TEST_ASSERT(ret != 0, "Should not have PERM_RUN permission");
    
    sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_sandbox_grant_revoke(void) {
    Sandbox sb;
    sandbox_init(&sb, 1, PERM_SEND);
    
    int ret = sandbox_grant_perm(&sb, PERM_RECV);
    TEST_ASSERT(ret == ERR_OK, "sandbox_grant_perm should succeed");
    TEST_ASSERT((sb.perm & PERM_RECV) != 0, "Should have PERM_RECV after grant");
    
    ret = sandbox_revoke_perm(&sb, PERM_SEND);
    TEST_ASSERT(ret == ERR_OK, "sandbox_revoke_perm should succeed");
    TEST_ASSERT((sb.perm & PERM_SEND) == 0, "Should not have PERM_SEND after revoke");
    
    sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_sandbox_resource_management(void) {
    Sandbox sb;
    sandbox_init(&sb, 1, PERM_SEND | PERM_RECV);
    
    int ret = sandbox_set_resource_limit(&sb, RESOURCE_TYPE_MEMORY, 1024);
    TEST_ASSERT(ret == ERR_OK, "sandbox_set_resource_limit should succeed");
    
    ret = sandbox_resource_alloc(&sb, RESOURCE_TYPE_MEMORY, 512);
    TEST_ASSERT(ret == ERR_OK, "sandbox_resource_alloc should succeed");
    TEST_ASSERT(sb.resources[RESOURCE_TYPE_MEMORY].used == 512, "Used memory should be 512");
    
    uint64_t peak = sandbox_get_resource_peak(&sb, RESOURCE_TYPE_MEMORY);
    TEST_ASSERT(peak == 512, "Peak memory should be 512");
    
    ret = sandbox_resource_free(&sb, RESOURCE_TYPE_MEMORY, 256);
    TEST_ASSERT(ret == ERR_OK, "sandbox_resource_free should succeed");
    TEST_ASSERT(sb.resources[RESOURCE_TYPE_MEMORY].used == 256, "Used memory should be 256");
    
    sandbox_reset_stats(&sb);
    TEST_ASSERT(sb.resources[RESOURCE_TYPE_MEMORY].peak == 256, "Peak should be reset to 256");
    
    sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_sandbox_resource_limit_exceed(void) {
    Sandbox sb;
    sandbox_init(&sb, 1, 0);
    
    sandbox_set_resource_limit(&sb, RESOURCE_TYPE_MEMORY, 100);
    
    int ret = sandbox_resource_check(&sb, RESOURCE_TYPE_MEMORY, 50);
    TEST_ASSERT(ret == ERR_OK, "sandbox_resource_check should succeed for 50");
    
    ret = sandbox_resource_alloc(&sb, RESOURCE_TYPE_MEMORY, 80);
    TEST_ASSERT(ret == ERR_OK, "sandbox_resource_alloc should succeed for 80");
    
    ret = sandbox_resource_check(&sb, RESOURCE_TYPE_MEMORY, 50);
    TEST_ASSERT(ret != ERR_OK, "sandbox_resource_check should fail for 50 when 80 used");
    
    sandbox_destroy(&sb);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Sandbox Tests");
    
    test_suite_add_test(&g_suite, "sandbox_init_destroy", test_sandbox_init_destroy);
    test_suite_add_test(&g_suite, "sandbox_perm_check", test_sandbox_perm_check);
    test_suite_add_test(&g_suite, "sandbox_grant_revoke", test_sandbox_grant_revoke);
    test_suite_add_test(&g_suite, "sandbox_resource_management", test_sandbox_resource_management);
    test_suite_add_test(&g_suite, "sandbox_resource_limit_exceed", test_sandbox_resource_limit_exceed);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
