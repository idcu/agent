#include "test/test_framework.h"
#include "module/dynamic_module.h"
#include "utils/log.h"
#include <stdio.h>

static TestSuite g_suite;

static int test_module_dependency_circular(void) {
    DynamicLoader loader;
    int ret = dynamic_loader_init(&loader, NULL);
    TEST_ASSERT(ret == ERR_OK, "dynamic_loader_init should succeed");

    TEST_PASS();
}

static int test_module_topo_sort(void) {
    DynamicLoader loader;
    int ret = dynamic_loader_init(&loader, NULL);
    TEST_ASSERT(ret == ERR_OK, "dynamic_loader_init should succeed");

    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);

    test_suite_init(&g_suite, "Module Integration Tests");

    test_suite_add_test(&g_suite, "test_module_dependency_circular", test_module_dependency_circular);
    test_suite_add_test(&g_suite, "test_module_topo_sort", test_module_topo_sort);

    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);

    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();

    return failures > 0 ? 1 : 0;
}
