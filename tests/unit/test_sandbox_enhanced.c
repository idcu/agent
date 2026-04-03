#include "test_framework.h"
#include "sandbox_enhanced.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static TestSuite g_suite;

static void test_enhanced_sandbox_init_destroy(void) {
    EnhancedSandbox sb;
    int ret = enhanced_sandbox_init(&sb, 1, PERM_SEND | PERM_RECV);
    TEST_ASSERT(ret == ERR_OK, "enhanced_sandbox_init should succeed");
    TEST_ASSERT(sb.base.module_id == 1, "Module ID should be 1");
    TEST_ASSERT(sb.base.active != 0, "Sandbox should be active");
    TEST_ASSERT(sb.mem_region_count == 0, "Memory region count should be 0");
    TEST_ASSERT(sb.syscall_count == 0, "Syscall count should be 0");
    
    enhanced_sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_enhanced_sandbox_memory_regions(void) {
    EnhancedSandbox sb;
    enhanced_sandbox_init(&sb, 1, 0);
    
    void* test_addr1 = malloc(1024);
    void* test_addr2 = malloc(2048);
    
    int ret = enhanced_sandbox_add_memory_region(&sb, test_addr1, 1024, MEM_REGION_READ | MEM_REGION_WRITE);
    TEST_ASSERT(ret == ERR_OK, "Add memory region 1 should succeed");
    TEST_ASSERT(sb.mem_region_count == 1, "Memory region count should be 1");
    
    ret = enhanced_sandbox_add_memory_region(&sb, test_addr2, 2048, MEM_REGION_READ);
    TEST_ASSERT(ret == ERR_OK, "Add memory region 2 should succeed");
    TEST_ASSERT(sb.mem_region_count == 2, "Memory region count should be 2");
    
    ret = enhanced_sandbox_check_memory_access(&sb, test_addr1, 512, MEM_REGION_READ);
    TEST_ASSERT(ret == ERR_OK, "Read access to region 1 should succeed");
    
    ret = enhanced_sandbox_check_memory_access(&sb, test_addr1, 512, MEM_REGION_WRITE);
    TEST_ASSERT(ret == ERR_OK, "Write access to region 1 should succeed");
    
    ret = enhanced_sandbox_check_memory_access(&sb, test_addr2, 1024, MEM_REGION_READ);
    TEST_ASSERT(ret == ERR_OK, "Read access to region 2 should succeed");
    
    ret = enhanced_sandbox_check_memory_access(&sb, test_addr2, 1024, MEM_REGION_WRITE);
    TEST_ASSERT(ret != ERR_OK, "Write access to region 2 should fail");
    
    ret = enhanced_sandbox_remove_memory_region(&sb, test_addr1);
    TEST_ASSERT(ret == ERR_OK, "Remove memory region 1 should succeed");
    TEST_ASSERT(sb.mem_region_count == 1, "Memory region count should be 1");
    
    free(test_addr1);
    free(test_addr2);
    enhanced_sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_enhanced_sandbox_syscall_whitelist(void) {
    EnhancedSandbox sb;
    enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = enhanced_sandbox_add_syscall(&sb, 1, "read");
    TEST_ASSERT(ret == ERR_OK, "Add syscall 1 should succeed");
    TEST_ASSERT(sb.syscall_count == 1, "Syscall count should be 1");
    
    ret = enhanced_sandbox_add_syscall(&sb, 2, "write");
    TEST_ASSERT(ret == ERR_OK, "Add syscall 2 should succeed");
    TEST_ASSERT(sb.syscall_count == 2, "Syscall count should be 2");
    
    ret = enhanced_sandbox_check_syscall(&sb, 1);
    TEST_ASSERT(ret == ERR_OK, "Check syscall 1 should succeed");
    
    ret = enhanced_sandbox_check_syscall(&sb, 2);
    TEST_ASSERT(ret == ERR_OK, "Check syscall 2 should succeed");
    
    ret = enhanced_sandbox_check_syscall(&sb, 3);
    TEST_ASSERT(ret != ERR_OK, "Check syscall 3 should fail");
    
    ret = enhanced_sandbox_remove_syscall(&sb, 1);
    TEST_ASSERT(ret == ERR_OK, "Remove syscall 1 should succeed");
    TEST_ASSERT(sb.syscall_count == 1, "Syscall count should be 1");
    
    ret = enhanced_sandbox_check_syscall(&sb, 1);
    TEST_ASSERT(ret != ERR_OK, "Check syscall 1 after remove should fail");
    
    enhanced_sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_enhanced_sandbox_resource_limits(void) {
    EnhancedSandbox sb;
    enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = enhanced_sandbox_set_cpu_limit(&sb, 1000);
    TEST_ASSERT(ret == ERR_OK, "Set CPU limit should succeed");
    TEST_ASSERT(sb.max_cpu_time_ms == 1000, "CPU limit should be 1000");
    
    ret = enhanced_sandbox_set_memory_limit(&sb, 1024 * 1024);
    TEST_ASSERT(ret == ERR_OK, "Set memory limit should succeed");
    TEST_ASSERT(sb.max_memory_bytes == 1024 * 1024, "Memory limit should be 1MB");
    
    ret = enhanced_sandbox_set_fd_limit(&sb, 100);
    TEST_ASSERT(ret == ERR_OK, "Set FD limit should succeed");
    TEST_ASSERT(sb.max_file_descriptors == 100, "FD limit should be 100");
    
    ret = enhanced_sandbox_check_cpu_usage(&sb);
    TEST_ASSERT(ret == ERR_OK, "Check CPU usage should succeed");
    
    ret = enhanced_sandbox_check_fd_usage(&sb, 50);
    TEST_ASSERT(ret == ERR_OK, "Check FD usage for 50 should succeed");
    
    enhanced_sandbox_destroy(&sb);
    TEST_PASS();
}

static void test_enhanced_sandbox_inheritance(void) {
    EnhancedSandbox sb;
    int ret = enhanced_sandbox_init(&sb, 1, PERM_SEND | PERM_RECV);
    TEST_ASSERT(ret == ERR_OK, "enhanced_sandbox_init should succeed");
    
    TEST_ASSERT(sb.base.module_id == 1, "Base module ID should be 1");
    TEST_ASSERT((sb.base.perm & PERM_SEND) != 0, "Should have PERM_SEND");
    TEST_ASSERT((sb.base.perm & PERM_RECV) != 0, "Should have PERM_RECV");
    
    enhanced_sandbox_destroy(&sb);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Enhanced Sandbox Tests");
    
    test_suite_add_test(&g_suite, "enhanced_sandbox_init_destroy", test_enhanced_sandbox_init_destroy);
    test_suite_add_test(&g_suite, "enhanced_sandbox_memory_regions", test_enhanced_sandbox_memory_regions);
    test_suite_add_test(&g_suite, "enhanced_sandbox_syscall_whitelist", test_enhanced_sandbox_syscall_whitelist);
    test_suite_add_test(&g_suite, "enhanced_sandbox_resource_limits", test_enhanced_sandbox_resource_limits);
    test_suite_add_test(&g_suite, "enhanced_sandbox_inheritance", test_enhanced_sandbox_inheritance);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
