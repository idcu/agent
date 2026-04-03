#include "test/test_framework.h"
#include "security/sandbox_enhanced.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static idcu_TestSuite g_suite;

static void test_enhanced_sandbox_init_destroy(void) {
    idcu_EnhancedSandbox sb;
    int ret = idcu_enhanced_sandbox_init(&sb, 1, IDCU_PERM_SEND | IDCU_PERM_RECV);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_enhanced_sandbox_init should succeed");
    IDCU_TEST_ASSERT(sb.base.module_id == 1, "Module ID should be 1");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_memory_regions(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    void* test_addr1 = malloc(1024);
    void* test_addr2 = malloc(2048);
    
    int ret = idcu_enhanced_sandbox_add_memory_region(&sb, test_addr1, 1024, IDCU_MEM_REGION_READ | IDCU_MEM_REGION_WRITE);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add memory region 1 should succeed");
    
    ret = idcu_enhanced_sandbox_add_memory_region(&sb, test_addr2, 2048, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add memory region 2 should succeed");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, test_addr1, 512, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Read access to region 1 should succeed");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, test_addr1, 512, IDCU_MEM_REGION_WRITE);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Write access to region 1 should succeed");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, test_addr2, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Read access to region 2 should succeed");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, test_addr2, 1024, IDCU_MEM_REGION_WRITE);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK, "Write access to region 2 should fail");
    
    ret = idcu_enhanced_sandbox_remove_memory_region(&sb, test_addr1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove memory region 1 should succeed");
    
    free(test_addr1);
    free(test_addr2);
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_syscall_whitelist(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_enhanced_sandbox_add_syscall(&sb, 1, "read");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add syscall 1 should succeed");
    
    ret = idcu_enhanced_sandbox_add_syscall(&sb, 2, "write");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add syscall 2 should succeed");
    
    ret = idcu_enhanced_sandbox_check_syscall(&sb, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check syscall 1 should succeed");
    
    ret = idcu_enhanced_sandbox_check_syscall(&sb, 2);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check syscall 2 should succeed");
    
    ret = idcu_enhanced_sandbox_check_syscall(&sb, 3);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK, "Check syscall 3 should fail");
    
    ret = idcu_enhanced_sandbox_remove_syscall(&sb, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove syscall 1 should succeed");
    
    ret = idcu_enhanced_sandbox_check_syscall(&sb, 1);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_OK, "Check syscall 1 after remove should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_resource_limits(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_enhanced_sandbox_set_cpu_limit(&sb, 1000);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set CPU limit should succeed");
    
    ret = idcu_enhanced_sandbox_set_memory_limit(&sb, 1024 * 1024);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set memory limit should succeed");
    
    ret = idcu_enhanced_sandbox_set_fd_limit(&sb, 100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set FD limit should succeed");
    
    ret = idcu_enhanced_sandbox_check_cpu_usage(&sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check CPU usage should succeed");
    
    ret = idcu_enhanced_sandbox_check_fd_usage(&sb, 50);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check FD usage for 50 should succeed");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Enhanced Sandbox Tests");
    
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_init_destroy", test_enhanced_sandbox_init_destroy);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_memory_regions", test_enhanced_sandbox_memory_regions);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_syscall_whitelist", test_enhanced_sandbox_syscall_whitelist);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_resource_limits", test_enhanced_sandbox_resource_limits);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
