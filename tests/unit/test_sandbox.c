#include "test/test_framework.h"
#include "security/sandbox.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_sandbox_init_destroy(void) {
    idcu_Sandbox sb;
    sb.module_id = 1;
    sb.perm = IDCU_PERM_SEND | IDCU_PERM_RECV;
    IDCU_TEST_ASSERT(sb.module_id == 1, "Module ID should be 1");
    IDCU_TEST_ASSERT(idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND) == 0, "Should have SEND permission");
    IDCU_TEST_PASS();
}

static void test_sandbox_perm_check(void) {
    idcu_Sandbox sb;
    sb.module_id = 1;
    sb.perm = IDCU_PERM_SEND | IDCU_PERM_RECV;
    
    int ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_SEND);
    IDCU_TEST_ASSERT(ret == 0, "Should have PERM_SEND permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RECV);
    IDCU_TEST_ASSERT(ret == 0, "Should have PERM_RECV permission");
    
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_RUN);
    IDCU_TEST_ASSERT(ret != 0, "Should not have PERM_RUN permission");
    
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Sandbox Tests");
    
    idcu_test_suite_add_test(&g_suite, "sandbox_init_destroy", test_sandbox_init_destroy);
    idcu_test_suite_add_test(&g_suite, "sandbox_perm_check", test_sandbox_perm_check);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
