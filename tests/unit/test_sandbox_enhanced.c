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
    IDCU_TEST_ASSERT(sb.active == 1, "Sandbox should be active after init");
    IDCU_TEST_ASSERT(sb.mem_region_count == 0, "mem_region_count should be 0");
    IDCU_TEST_ASSERT(sb.syscall_count == 0, "syscall_count should be 0");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_init_null_param(void) {
    int ret = idcu_enhanced_sandbox_init(NULL, 1, 0);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "idcu_enhanced_sandbox_init with NULL should fail");
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
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Write access to region 2 should fail");
    
    ret = idcu_enhanced_sandbox_remove_memory_region(&sb, test_addr1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove memory region 1 should succeed");
    
    free(test_addr1);
    free(test_addr2);
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_memory_region_null_params(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_enhanced_sandbox_add_memory_region(NULL, (void*)0x1000, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "add_memory_region with NULL sb should fail");
    
    ret = idcu_enhanced_sandbox_add_memory_region(&sb, NULL, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "add_memory_region with NULL addr should fail");
    
    ret = idcu_enhanced_sandbox_add_memory_region(&sb, (void*)0x1000, 0, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "add_memory_region with size 0 should fail");
    
    ret = idcu_enhanced_sandbox_check_memory_access(NULL, (void*)0x1000, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_memory_access with NULL sb should fail");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, NULL, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_memory_access with NULL addr should fail");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, (void*)0x1000, 0, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_memory_access with size 0 should fail");
    
    ret = idcu_enhanced_sandbox_remove_memory_region(NULL, (void*)0x1000);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "remove_memory_region with NULL sb should fail");
    
    ret = idcu_enhanced_sandbox_remove_memory_region(&sb, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "remove_memory_region with NULL addr should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_memory_region_not_found(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    void* test_addr = malloc(1024);
    
    int ret = idcu_enhanced_sandbox_remove_memory_region(&sb, test_addr);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "remove_memory_region for non-existent region should fail");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, test_addr, 512, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "check_memory_access for non-existent region should fail");
    
    free(test_addr);
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_memory_regions_full(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    void* addrs[IDCU_MAX_MEMORY_REGIONS];
    for (int i = 0; i < IDCU_MAX_MEMORY_REGIONS; i++) {
        addrs[i] = malloc(1024);
    }
    
    int all_succeeded = 1;
    for (int i = 0; i < IDCU_MAX_MEMORY_REGIONS; i++) {
        int ret = idcu_enhanced_sandbox_add_memory_region(&sb, addrs[i], 1024, IDCU_MEM_REGION_READ);
        if (ret != IDCU_ERR_OK) {
            all_succeeded = 0;
            break;
        }
    }
    IDCU_TEST_ASSERT(all_succeeded, "All memory regions should be added successfully");
    
    void* extra_addr = malloc(1024);
    int ret = idcu_enhanced_sandbox_add_memory_region(&sb, extra_addr, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NO_MEMORY, "Add extra memory region should fail");
    
    free(extra_addr);
    for (int i = 0; i < IDCU_MAX_MEMORY_REGIONS; i++) {
        free(addrs[i]);
    }
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_memory_region_partial_access(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    void* test_addr = malloc(1024);
    
    idcu_enhanced_sandbox_add_memory_region(&sb, test_addr, 1024, IDCU_MEM_REGION_READ);
    
    int ret = idcu_enhanced_sandbox_check_memory_access(&sb, test_addr, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Full read access should succeed");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, (void*)((uintptr_t)test_addr - 1), 10, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Access before region should fail");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, (void*)((uintptr_t)test_addr + 1000), 100, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Access after region should fail");
    
    free(test_addr);
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
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Check syscall 3 should fail");
    
    ret = idcu_enhanced_sandbox_remove_syscall(&sb, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove syscall 1 should succeed");
    
    ret = idcu_enhanced_sandbox_check_syscall(&sb, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Check syscall 1 after remove should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_syscall_null_params(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_enhanced_sandbox_add_syscall(NULL, 1, "read");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "add_syscall with NULL sb should fail");
    
    ret = idcu_enhanced_sandbox_add_syscall(&sb, 1, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "add_syscall with NULL name should fail");
    
    ret = idcu_enhanced_sandbox_check_syscall(NULL, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_syscall with NULL sb should fail");
    
    ret = idcu_enhanced_sandbox_remove_syscall(NULL, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "remove_syscall with NULL sb should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_syscall_not_found(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_enhanced_sandbox_remove_syscall(&sb, 999);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "remove_syscall for non-existent should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_syscalls_full(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int all_succeeded = 1;
    for (int i = 0; i < IDCU_MAX_SYSCALLS; i++) {
        char name[32];
        snprintf(name, sizeof(name), "syscall%d", i);
        int ret = idcu_enhanced_sandbox_add_syscall(&sb, i, name);
        if (ret != IDCU_ERR_OK) {
            all_succeeded = 0;
            break;
        }
    }
    IDCU_TEST_ASSERT(all_succeeded, "All syscalls should be added successfully");
    
    int ret = idcu_enhanced_sandbox_add_syscall(&sb, IDCU_MAX_SYSCALLS, "extra");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NO_MEMORY, "Add extra syscall should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_resource_limits(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_enhanced_sandbox_set_cpu_limit(&sb, 1000);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set CPU limit should succeed");
    IDCU_TEST_ASSERT(sb.max_cpu_time_ms == 1000, "CPU limit should be 1000");
    
    ret = idcu_enhanced_sandbox_set_memory_limit(&sb, 1024 * 1024);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set memory limit should succeed");
    IDCU_TEST_ASSERT(sb.max_memory_bytes == 1024 * 1024, "Memory limit should be 1MB");
    
    ret = idcu_enhanced_sandbox_set_fd_limit(&sb, 100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Set FD limit should succeed");
    IDCU_TEST_ASSERT(sb.max_file_descriptors == 100, "FD limit should be 100");
    
    ret = idcu_enhanced_sandbox_check_cpu_usage(&sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check CPU usage should succeed when within limit");
    
    ret = idcu_enhanced_sandbox_check_fd_usage(&sb, 50);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check FD usage for 50 should succeed");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_resource_limit_null_params(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    int ret = idcu_enhanced_sandbox_set_cpu_limit(NULL, 1000);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "set_cpu_limit with NULL should fail");
    
    ret = idcu_enhanced_sandbox_set_memory_limit(NULL, 1024);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "set_memory_limit with NULL should fail");
    
    ret = idcu_enhanced_sandbox_set_fd_limit(NULL, 100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "set_fd_limit with NULL should fail");
    
    ret = idcu_enhanced_sandbox_check_cpu_usage(NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_cpu_usage with NULL should fail");
    
    ret = idcu_enhanced_sandbox_check_fd_usage(NULL, 50);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_fd_usage with NULL should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_cpu_limit_exceeded(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    idcu_enhanced_sandbox_set_cpu_limit(&sb, 1000);
    sb.used_cpu_time_ms = 999;
    
    int ret = idcu_enhanced_sandbox_check_cpu_usage(&sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check CPU usage at 999 should succeed");
    
    sb.used_cpu_time_ms = 1000;
    ret = idcu_enhanced_sandbox_check_cpu_usage(&sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Check CPU usage at 1000 should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_fd_limit_exceeded(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    idcu_enhanced_sandbox_set_fd_limit(&sb, 100);
    sb.used_file_descriptors = 50;
    
    int ret = idcu_enhanced_sandbox_check_fd_usage(&sb, 49);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check FD usage for 49 should succeed");
    
    ret = idcu_enhanced_sandbox_check_fd_usage(&sb, 51);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_PERM_DENIED, "Check FD usage for 51 should fail");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_zero_limits(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    
    idcu_enhanced_sandbox_set_cpu_limit(&sb, 0);
    int ret = idcu_enhanced_sandbox_check_cpu_usage(&sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check CPU with zero limit should always succeed");
    
    idcu_enhanced_sandbox_set_fd_limit(&sb, 0);
    ret = idcu_enhanced_sandbox_check_fd_usage(&sb, 9999);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Check FD with zero limit should always succeed");
    
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

static void test_enhanced_sandbox_inactive(void) {
    idcu_EnhancedSandbox sb;
    idcu_enhanced_sandbox_init(&sb, 1, 0);
    sb.active = 0;
    
    void* test_addr = malloc(1024);
    
    int ret = idcu_enhanced_sandbox_add_memory_region(&sb, test_addr, 1024, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "add_memory_region on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_remove_memory_region(&sb, test_addr);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "remove_memory_region on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_check_memory_access(&sb, test_addr, 512, IDCU_MEM_REGION_READ);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_memory_access on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_add_syscall(&sb, 1, "read");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "add_syscall on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_remove_syscall(&sb, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "remove_syscall on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_check_syscall(&sb, 1);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_syscall on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_set_cpu_limit(&sb, 1000);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "set_cpu_limit on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_set_memory_limit(&sb, 1024);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "set_memory_limit on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_set_fd_limit(&sb, 100);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "set_fd_limit on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_check_cpu_usage(&sb);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_cpu_usage on inactive sb should fail");
    
    ret = idcu_enhanced_sandbox_check_fd_usage(&sb, 50);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "check_fd_usage on inactive sb should fail");
    
    free(test_addr);
    idcu_enhanced_sandbox_destroy(&sb);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Enhanced Sandbox Tests");
    
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_init_destroy", test_enhanced_sandbox_init_destroy);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_init_null_param", test_enhanced_sandbox_init_null_param);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_memory_regions", test_enhanced_sandbox_memory_regions);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_memory_region_null_params", test_enhanced_sandbox_memory_region_null_params);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_memory_region_not_found", test_enhanced_sandbox_memory_region_not_found);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_memory_regions_full", test_enhanced_sandbox_memory_regions_full);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_memory_region_partial_access", test_enhanced_sandbox_memory_region_partial_access);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_syscall_whitelist", test_enhanced_sandbox_syscall_whitelist);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_syscall_null_params", test_enhanced_sandbox_syscall_null_params);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_syscall_not_found", test_enhanced_sandbox_syscall_not_found);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_syscalls_full", test_enhanced_sandbox_syscalls_full);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_resource_limits", test_enhanced_sandbox_resource_limits);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_resource_limit_null_params", test_enhanced_sandbox_resource_limit_null_params);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_cpu_limit_exceeded", test_enhanced_sandbox_cpu_limit_exceeded);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_fd_limit_exceeded", test_enhanced_sandbox_fd_limit_exceeded);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_zero_limits", test_enhanced_sandbox_zero_limits);
    idcu_test_suite_add_test(&g_suite, "enhanced_sandbox_inactive", test_enhanced_sandbox_inactive);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
