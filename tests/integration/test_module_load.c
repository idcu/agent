#include "test_framework.h"
#include "micro_kernel.h"
#include "module_registry.h"
#include "module_def.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_MicroKernel g_kernel;

idcu_MicroKernel* idcu_get_kernel(void)
{
    return &g_kernel;
}

static idcu_TestSuite g_suite;

static int test_module_a_init(void) { return IDCU_ERR_OK; }
static int test_module_a_run(void) { return IDCU_ERR_OK; }
static int test_module_a_stop(void) { return IDCU_ERR_OK; }

static int test_module_b_init(void) { return IDCU_ERR_OK; }
static int test_module_b_run(void) { return IDCU_ERR_OK; }
static int test_module_b_stop(void) { return IDCU_ERR_OK; }

static int test_module_c_init(void) { return IDCU_ERR_OK; }
static int test_module_c_run(void) { return IDCU_ERR_OK; }
static int test_module_c_stop(void) { return IDCU_ERR_OK; }

static const idcu_ModuleDependency g_module_b_deps[] = {
    {"test_module_a"}
};

static const idcu_ModuleDependency g_module_c_deps[] = {
    {"test_module_a"},
    {"test_module_b"}
};

static const idcu_ModuleInterface g_test_module_a = {
    .name = "test_module_a",
    .dependencies = NULL,
    .dependency_count = 0,
    .init = test_module_a_init,
    .run = test_module_a_run,
    .stop = test_module_a_stop
};

static const idcu_ModuleInterface g_test_module_b = {
    .name = "test_module_b",
    .dependencies = g_module_b_deps,
    .dependency_count = 1,
    .init = test_module_b_init,
    .run = test_module_b_run,
    .stop = test_module_b_stop
};

static const idcu_ModuleInterface g_test_module_c = {
    .name = "test_module_c",
    .dependencies = g_module_c_deps,
    .dependency_count = 2,
    .init = test_module_c_init,
    .run = test_module_c_run,
    .stop = test_module_c_stop
};

static void test_module_dependency_graph_build(void) {
    idcu_ModuleRegistry reg;
    int ret = idcu_module_registry_init(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "registry_init should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_a, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_a should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_b, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_b should succeed");
    
    ret = idcu_module_registry_build_dependency_graph(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "build_dependency_graph should succeed");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_module_topological_sort(void) {
    idcu_ModuleRegistry reg;
    int ret = idcu_module_registry_init(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "registry_init should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_c, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_c should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_a, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_a should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_b, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_b should succeed");
    
    ret = idcu_module_registry_build_dependency_graph(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "build_dependency_graph should succeed");
    
    ret = idcu_module_registry_topological_sort(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "topological_sort should succeed");
    
    const idcu_RegisteredModule* mod_a = idcu_module_registry_find_by_name(&reg, "test_module_a");
    const idcu_RegisteredModule* mod_b = idcu_module_registry_find_by_name(&reg, "test_module_b");
    const idcu_RegisteredModule* mod_c = idcu_module_registry_find_by_name(&reg, "test_module_c");
    
    IDCU_TEST_ASSERT(mod_a != NULL, "module_a should be found");
    IDCU_TEST_ASSERT(mod_b != NULL, "module_b should be found");
    IDCU_TEST_ASSERT(mod_c != NULL, "module_c should be found");
    
    IDCU_TEST_ASSERT(reg.topological_valid == 1, "topological should be valid");
    IDCU_TEST_ASSERT(reg.topological_count == 3, "should have 3 modules in order");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_module_init_order(void) {
    idcu_ModuleRegistry reg;
    int ret = idcu_module_registry_init(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "registry_init should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_c, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_c should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_b, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_b should succeed");
    
    ret = idcu_module_registry_register(&reg, &g_test_module_a, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register module_a should succeed");
    
    ret = idcu_module_registry_init_all(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "init_all should succeed");
    
    ret = idcu_module_registry_run_all(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "run_all should succeed");
    
    ret = idcu_module_registry_stop_all(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "stop_all should succeed");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_module_dependency_circular(void) {
    idcu_ModuleRegistry reg;
    int ret = idcu_module_registry_init(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "registry_init should succeed");
    
    idcu_ModuleDependency circular_deps_a[] = {{"circular_b"}};
    idcu_ModuleDependency circular_deps_b[] = {{"circular_a"}};
    
    idcu_ModuleInterface circular_a = {
        .name = "circular_a",
        .dependencies = circular_deps_a,
        .dependency_count = 1,
        .init = test_module_a_init,
        .run = test_module_a_run,
        .stop = test_module_a_stop
    };
    
    idcu_ModuleInterface circular_b = {
        .name = "circular_b",
        .dependencies = circular_deps_b,
        .dependency_count = 1,
        .init = test_module_b_init,
        .run = test_module_b_run,
        .stop = test_module_b_stop
    };
    
    ret = idcu_module_registry_register(&reg, &circular_a, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register circular_a should succeed");
    
    ret = idcu_module_registry_register(&reg, &circular_b, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register circular_b should succeed");
    
    ret = idcu_module_registry_build_dependency_graph(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "build_dependency_graph should succeed");
    
    ret = idcu_module_registry_topological_sort(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_CIRCULAR_DEP, "topological_sort should detect circular dependency");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

static void test_module_missing_dependency(void) {
    idcu_ModuleRegistry reg;
    int ret = idcu_module_registry_init(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "registry_init should succeed");
    
    idcu_ModuleDependency missing_dep[] = {{"nonexistent_module"}};
    
    idcu_ModuleInterface module_with_missing_dep = {
        .name = "module_with_missing_dep",
        .dependencies = missing_dep,
        .dependency_count = 1,
        .init = test_module_a_init,
        .run = test_module_a_run,
        .stop = test_module_a_stop
    };
    
    ret = idcu_module_registry_register(&reg, &module_with_missing_dep, IDCU_MOD_PRIO_NORMAL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "register should succeed");
    
    ret = idcu_module_registry_build_dependency_graph(&reg);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "build_dependency_graph should detect missing dependency");
    
    idcu_module_registry_destroy(&reg);
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_test_suite_init(&g_suite, "Module Load Integration Tests");

    idcu_test_suite_add_test(&g_suite, "module_dependency_graph_build", test_module_dependency_graph_build);
    idcu_test_suite_add_test(&g_suite, "module_topological_sort", test_module_topological_sort);
    idcu_test_suite_add_test(&g_suite, "module_init_order", test_module_init_order);
    idcu_test_suite_add_test(&g_suite, "module_dependency_circular", test_module_dependency_circular);
    idcu_test_suite_add_test(&g_suite, "module_missing_dependency", test_module_missing_dependency);

    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);

    int failures = idcu_test_suite_get_failures(&g_suite);

    return failures > 0 ? 1 : 0;
}
