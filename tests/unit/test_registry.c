#include "test_framework.h"
#include "module_registry.h"
#include "module_def.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static const ModuleMeta g_test_meta1 = {
    .name = "test_module1",
    .version = "1.0.0",
    .author = "test",
    .description = "Test module 1"
};

static const ModuleMeta g_test_meta2 = {
    .name = "test_module2",
    .version = "1.0.0",
    .author = "test",
    .description = "Test module 2"
};

static int test_module1_init(void* data) { return ERR_OK; }
static int test_module1_run(void* data) { return ERR_OK; }
static int test_module1_stop(void* data) { return ERR_OK; }

static int test_module2_init(void* data) { return ERR_OK; }
static int test_module2_run(void* data) { return ERR_OK; }
static int test_module2_stop(void* data) { return ERR_OK; }

static const ModuleInterface g_test_iface1 = {
    .meta = &g_test_meta1,
    .init = test_module1_init,
    .run = test_module1_run,
    .stop = test_module1_stop,
    .priority = 100,
    .user_data = NULL
};

static const ModuleInterface g_test_iface2 = {
    .meta = &g_test_meta2,
    .init = test_module2_init,
    .run = test_module2_run,
    .stop = test_module2_stop,
    .priority = 50,
    .user_data = NULL
};

static void test_registry_init_clear(void) {
    ModuleRegistry reg;
    int ret = registry_init(&reg);
    TEST_ASSERT(ret == ERR_OK, "registry_init should succeed");
    TEST_ASSERT(registry_get_count(&reg) == 0, "Initial count should be 0");
    
    registry_clear(&reg);
    TEST_PASS();
}

static void test_registry_register(void) {
    ModuleRegistry reg;
    registry_init(&reg);
    
    int ret = registry_register(&reg, &g_test_iface1);
    TEST_ASSERT(ret == ERR_OK, "registry_register should succeed");
    TEST_ASSERT(registry_get_count(&reg) == 1, "Count should be 1 after register");
    
    registry_clear(&reg);
    TEST_PASS();
}

static void test_registry_find_by_name(void) {
    ModuleRegistry reg;
    registry_init(&reg);
    
    registry_register(&reg, &g_test_iface1);
    registry_register(&reg, &g_test_iface2);
    
    const ModuleInterface* found = registry_find_by_name(&reg, "test_module1");
    TEST_ASSERT(found != NULL, "Should find test_module1");
    TEST_ASSERT(found == &g_test_iface1, "Found interface should match");
    
    found = registry_find_by_name(&reg, "nonexistent");
    TEST_ASSERT(found == NULL, "Should return NULL for nonexistent module");
    
    registry_clear(&reg);
    TEST_PASS();
}

static void test_registry_get_at(void) {
    ModuleRegistry reg;
    registry_init(&reg);
    
    registry_register(&reg, &g_test_iface1);
    registry_register(&reg, &g_test_iface2);
    
    const ModuleInterface* mod0 = registry_get_at(&reg, 0);
    TEST_ASSERT(mod0 != NULL, "Should get module at index 0");
    
    const ModuleInterface* mod1 = registry_get_at(&reg, 1);
    TEST_ASSERT(mod1 != NULL, "Should get module at index 1");
    
    const ModuleInterface* mod_invalid = registry_get_at(&reg, 999);
    TEST_ASSERT(mod_invalid == NULL, "Should return NULL for invalid index");
    
    registry_clear(&reg);
    TEST_PASS();
}

static void test_registry_sort_by_priority(void) {
    ModuleRegistry reg;
    registry_init(&reg);
    
    registry_register(&reg, &g_test_iface1);
    registry_register(&reg, &g_test_iface2);
    
    int ret = registry_sort_by_priority(&reg);
    TEST_ASSERT(ret == ERR_OK, "registry_sort_by_priority should succeed");
    
    registry_clear(&reg);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Module Registry Tests");
    
    test_suite_add_test(&g_suite, "registry_init_clear", test_registry_init_clear);
    test_suite_add_test(&g_suite, "registry_register", test_registry_register);
    test_suite_add_test(&g_suite, "registry_find_by_name", test_registry_find_by_name);
    test_suite_add_test(&g_suite, "registry_get_at", test_registry_get_at);
    test_suite_add_test(&g_suite, "registry_sort_by_priority", test_registry_sort_by_priority);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
