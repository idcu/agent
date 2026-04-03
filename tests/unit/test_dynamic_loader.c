#include "test/test_framework.h"
#include "module/dynamic_module.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_dynamic_loader_init_destroy(void) {
    idcu_DynamicLoader loader;
    int ret = idcu_dynamic_loader_init(&loader, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_dynamic_loader_init should succeed");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_empty_count(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    int count = idcu_dynamic_loader_get_count(&loader);
    IDCU_TEST_ASSERT(count == 0, "Count should be 0 for empty loader");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_find_nonexistent(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "nonexistent");
    IDCU_TEST_ASSERT(mod == NULL, "Should return NULL for nonexistent module");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_get_at_invalid(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_get_at(&loader, 0);
    IDCU_TEST_ASSERT(mod == NULL, "Should return NULL for invalid index");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Dynamic Loader Tests");
    
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_init_destroy", test_dynamic_loader_init_destroy);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_empty_count", test_dynamic_loader_empty_count);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_find_nonexistent", test_dynamic_loader_find_nonexistent);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_get_at_invalid", test_dynamic_loader_get_at_invalid);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
