#include "test/test_framework.h"
#include "module/dynamic_module.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_dynamic_loader_init_destroy(void) {
    DynamicLoader loader;
    int ret = dynamic_loader_init(&loader, NULL);
    TEST_ASSERT(ret == ERR_OK, "dynamic_loader_init should succeed");
    
    dynamic_loader_destroy(&loader);
    TEST_PASS();
}

static void test_dynamic_loader_empty_count(void) {
    DynamicLoader loader;
    dynamic_loader_init(&loader, NULL);
    
    int count = dynamic_loader_get_count(&loader);
    TEST_ASSERT(count == 0, "Count should be 0 for empty loader");
    
    dynamic_loader_destroy(&loader);
    TEST_PASS();
}

static void test_dynamic_loader_find_nonexistent(void) {
    DynamicLoader loader;
    dynamic_loader_init(&loader, NULL);
    
    DynamicModule* mod = dynamic_loader_find_module(&loader, "nonexistent");
    TEST_ASSERT(mod == NULL, "Should return NULL for nonexistent module");
    
    dynamic_loader_destroy(&loader);
    TEST_PASS();
}

static void test_dynamic_loader_get_at_invalid(void) {
    DynamicLoader loader;
    dynamic_loader_init(&loader, NULL);
    
    DynamicModule* mod = dynamic_loader_get_at(&loader, 0);
    TEST_ASSERT(mod == NULL, "Should return NULL for invalid index");
    
    dynamic_loader_destroy(&loader);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Dynamic Loader Tests");
    
    test_suite_add_test(&g_suite, "dynamic_loader_init_destroy", test_dynamic_loader_init_destroy);
    test_suite_add_test(&g_suite, "dynamic_loader_empty_count", test_dynamic_loader_empty_count);
    test_suite_add_test(&g_suite, "dynamic_loader_find_nonexistent", test_dynamic_loader_find_nonexistent);
    test_suite_add_test(&g_suite, "dynamic_loader_get_at_invalid", test_dynamic_loader_get_at_invalid);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
