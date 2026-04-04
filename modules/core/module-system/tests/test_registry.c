#include "test_framework.h"
#include "micro_kernel.h"
#include "module_registry.h"
#include "module_def.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static idcu_MicroKernel g_kernel;

idcu_MicroKernel* idcu_get_kernel(void)
{
    return &g_kernel;
}

static idcu_TestSuite g_suite;

static int test_module1_init(void) { return IDCU_ERR_OK; }
static int test_module1_run(void) { return IDCU_ERR_OK; }
static int test_module1_stop(void) { return IDCU_ERR_OK; }

static int test_module2_init(void) { return IDCU_ERR_OK; }
static int test_module2_run(void) { return IDCU_ERR_OK; }
static int test_module2_stop(void) { return IDCU_ERR_OK; }

static const idcu_ModuleInterface g_test_iface1 = {
    .name = "test_module1",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_module1_init,
    .run = test_module1_run,
    .stop = test_module1_stop
};

static const idcu_ModuleInterface g_test_iface2 = {
    .name = "test_module2",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_module2_init,
    .run = test_module2_run,
    .stop = test_module2_stop
};

static void test_registry_init_clear(void) {
    idcu_ModuleRegistry reg;
    int ret = idcu_module_registry_init(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_module_registry_init should succeed");
    IDCU_TEST_ASSERT(idcu_module_registry_get_count(&reg) == 0, "Initial count should be 0");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_registry_register(void) {
    idcu_ModuleRegistry reg;
    idcu_module_registry_init(&reg);
    
    int ret = idcu_module_registry_register(&reg, &g_test_iface1, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_module_registry_register should succeed");
    IDCU_TEST_ASSERT(idcu_module_registry_get_count(&reg) == 1, "Count should be 1 after register");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_registry_find_by_name(void) {
    idcu_ModuleRegistry reg;
    idcu_module_registry_init(&reg);
    
    idcu_module_registry_register(&reg, &g_test_iface1, IDCU_MOD_PRIO_NORMAL);
    idcu_module_registry_register(&reg, &g_test_iface2, IDCU_MOD_PRIO_NORMAL);
    
    const idcu_RegisteredModule* found = idcu_module_registry_find_by_name(&reg, "test_module1");
    IDCU_TEST_ASSERT(found != NULL, "Should find test_module1");
    IDCU_TEST_ASSERT(found->iface == &g_test_iface1, "Found interface should match");
    
    found = idcu_module_registry_find_by_name(&reg, "nonexistent");
    IDCU_TEST_ASSERT(found == NULL, "Should return NULL for nonexistent module");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_registry_get_at(void) {
    idcu_ModuleRegistry reg;
    idcu_module_registry_init(&reg);
    
    idcu_module_registry_register(&reg, &g_test_iface1, IDCU_MOD_PRIO_NORMAL);
    idcu_module_registry_register(&reg, &g_test_iface2, IDCU_MOD_PRIO_NORMAL);
    
    const idcu_RegisteredModule* mod0 = idcu_module_registry_get_at(&reg, 0);
    IDCU_TEST_ASSERT(mod0 != NULL, "Should get module at index 0");
    
    const idcu_RegisteredModule* mod1 = idcu_module_registry_get_at(&reg, 1);
    IDCU_TEST_ASSERT(mod1 != NULL, "Should get module at index 1");
    
    const idcu_RegisteredModule* mod_invalid = idcu_module_registry_get_at(&reg, 999);
    IDCU_TEST_ASSERT(mod_invalid == NULL, "Should return NULL for invalid index");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_registry_dependency_graph(void) {
    idcu_ModuleRegistry reg;
    idcu_module_registry_init(&reg);
    
    idcu_module_registry_register(&reg, &g_test_iface1, IDCU_MOD_PRIO_NORMAL);
    idcu_module_registry_register(&reg, &g_test_iface2, IDCU_MOD_PRIO_NORMAL);
    
    int ret = idcu_module_registry_build_dependency_graph(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_module_registry_build_dependency_graph should succeed");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Module Registry Tests");
    
    idcu_test_suite_add_test(&g_suite, "registry_init_clear", test_registry_init_clear);
    idcu_test_suite_add_test(&g_suite, "registry_register", test_registry_register);
    idcu_test_suite_add_test(&g_suite, "registry_find_by_name", test_registry_find_by_name);
    idcu_test_suite_add_test(&g_suite, "registry_get_at", test_registry_get_at);
    idcu_test_suite_add_test(&g_suite, "registry_dependency_graph", test_registry_dependency_graph);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
