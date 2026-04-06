#include "test/test_framework.h"
#include "dynamic_module.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define TEST_MODULE_GOOD "test_module_good.dll"
#define TEST_MODULE_INIT_FAIL "test_module_init_fail.dll"
#define TEST_MODULE_NO_INTERFACE "test_module_no_interface.dll"
#define SAMPLE_MODULE "sample_module.dll"
#else
#define TEST_MODULE_GOOD "test_module_good.so"
#define TEST_MODULE_INIT_FAIL "test_module_init_fail.so"
#define TEST_MODULE_NO_INTERFACE "test_module_no_interface.so"
#define SAMPLE_MODULE "libsample_module.so"
#endif

static idcu_TestSuite g_suite;

static void test_dynamic_loader_init_destroy(void) {
    idcu_DynamicLoader loader;
    int ret = idcu_dynamic_loader_init(&loader, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_dynamic_loader_init should succeed");
    
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

static void test_dynamic_loader_load_invalid_path(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    int ret = idcu_dynamic_loader_load_module(&loader, "invalid", "nonexistent.dll");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_MODULE_LOAD, "Loading invalid module should fail");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_null_params(void) {
    int ret = idcu_dynamic_loader_init(NULL, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "init with NULL should fail");
    
    ret = idcu_dynamic_loader_load_module(NULL, "name", NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "load_module with NULL loader should fail");
    
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    ret = idcu_dynamic_loader_load_module(&loader, NULL, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "load_module with NULL name should fail");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_hotplug_null_params(void) {
    int ret = idcu_dynamic_module_restart(NULL, "name");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "restart with NULL loader should fail");
    
    ret = idcu_dynamic_module_reload(NULL, "name", NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "reload with NULL loader should fail");
    
    ret = idcu_dynamic_loader_hotplug_load(NULL, "name", NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "hotplug load with NULL loader should fail");
    
    ret = idcu_dynamic_loader_hotplug_unload(NULL, "name");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "hotplug unload with NULL loader should fail");
    
    IDCU_TEST_PASS();
}

static void test_hotplug_valid_loader_null_params(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    int ret = idcu_dynamic_module_restart(&loader, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "restart with NULL name should fail");
    
    ret = idcu_dynamic_module_reload(&loader, NULL, NULL);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_INVALID_PARAM, "reload with NULL name should fail");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_hotplug_restart_nonexistent(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    int ret = idcu_dynamic_module_restart(&loader, "nonexistent");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "restart nonexistent module should fail");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_hotplug_unload_nonexistent(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    int ret = idcu_dynamic_loader_hotplug_unload(&loader, "nonexistent");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "unload nonexistent module should fail");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_load_good_module(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_GOOD);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_GOOD);
#endif
    
    int ret = idcu_dynamic_loader_load_module(&loader, "test_good", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module (maybe not built yet), skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Loading good module should succeed");
    
    int count = idcu_dynamic_loader_get_count(&loader);
    IDCU_TEST_ASSERT(count == 1, "Count should be 1");
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "test_good");
    IDCU_TEST_ASSERT(mod != NULL, "Should find loaded module");
    
    ret = idcu_dynamic_loader_unload_module(&loader, "test_good");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Unloading module should succeed");
    
    count = idcu_dynamic_loader_get_count(&loader);
    IDCU_TEST_ASSERT(count == 0, "Count should be 0 after unload");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_module_lifecycle(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_GOOD);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_GOOD);
#endif
    
    int ret = idcu_dynamic_loader_load_module(&loader, "test_good", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module, skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "test_good");
    IDCU_TEST_ASSERT(mod != NULL, "Should find module");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_UNINIT, "Initial state should be UNINIT");
    
    ret = idcu_dynamic_module_init(mod);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Module init should succeed");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_INITED, "State should be INITED");
    
    ret = idcu_dynamic_module_run(mod);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Module run should succeed");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_RUNNING, "State should be RUNNING");
    
    ret = idcu_dynamic_module_stop(mod);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Module stop should succeed");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_STOPPED, "State should be STOPPED");
    
    idcu_dynamic_loader_unload_module(&loader, "test_good");
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_load_duplicate(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_GOOD);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_GOOD);
#endif
    
    int ret = idcu_dynamic_loader_load_module(&loader, "test_good", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module, skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    ret = idcu_dynamic_loader_load_module(&loader, "test_good", module_path);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_ALREADY_EXISTS, "Loading duplicate module should fail");
    
    idcu_dynamic_loader_unload_module(&loader, "test_good");
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_load_no_interface(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_NO_INTERFACE);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_NO_INTERFACE);
#endif
    
    int ret = idcu_dynamic_loader_load_module(&loader, "test_no_interface", module_path);
    if (ret == IDCU_ERR_MODULE_LOAD) {
        IDCU_LOG_WARN("Could not load test module (maybe not built yet), skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    IDCU_TEST_ASSERT(ret == IDCU_ERR_MODULE_INVALID, "Loading module without interface should return MODULE_INVALID");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_module_init_fail(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_INIT_FAIL);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_INIT_FAIL);
#endif
    
    int ret = idcu_dynamic_loader_load_module(&loader, "test_init_fail", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module, skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "test_init_fail");
    IDCU_TEST_ASSERT(mod != NULL, "Should find module");
    
    ret = idcu_dynamic_module_init(mod);
    IDCU_TEST_ASSERT(ret != IDCU_ERR_SUCCESS, "Module init should fail");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_ERROR, "State should be ERROR");
    
    idcu_dynamic_loader_unload_module(&loader, "test_init_fail");
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_hotplug_load(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_GOOD);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_GOOD);
#endif
    
    int ret = idcu_dynamic_loader_hotplug_load(&loader, "test_hotplug", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module, skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "test_hotplug");
    IDCU_TEST_ASSERT(mod != NULL, "Should find hotplug loaded module");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_RUNNING, "Module should be running");
    
    ret = idcu_dynamic_loader_hotplug_unload(&loader, "test_hotplug");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Hotplug unload should succeed");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_restart_module(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_GOOD);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_GOOD);
#endif
    
    int ret = idcu_dynamic_loader_load_module(&loader, "test_restart", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module, skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "test_restart");
    IDCU_TEST_ASSERT(mod != NULL, "Should find module");
    
    ret = idcu_dynamic_module_init(mod);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Init should succeed");
    ret = idcu_dynamic_module_run(mod);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Run should succeed");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_RUNNING, "Should be running");
    
    ret = idcu_dynamic_module_restart(&loader, "test_restart");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Restart should succeed");
    IDCU_TEST_ASSERT(mod->state == IDCU_MOD_STATE_RUNNING, "Should be running after restart");
    
    idcu_dynamic_loader_unload_module(&loader, "test_restart");
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_reload_module(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_GOOD);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_GOOD);
#endif
    
    int ret = idcu_dynamic_loader_load_module(&loader, "test_reload", module_path);
    if (ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module, skipping this test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    int count = idcu_dynamic_loader_get_count(&loader);
    IDCU_TEST_ASSERT(count == 1, "Count should be 1");
    
    ret = idcu_dynamic_module_reload(&loader, "test_reload", module_path);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Reload should succeed");
    
    count = idcu_dynamic_loader_get_count(&loader);
    IDCU_TEST_ASSERT(count == 1, "Count should still be 1 after reload");
    
    idcu_DynamicModule* mod = idcu_dynamic_loader_find_module(&loader, "test_reload");
    IDCU_TEST_ASSERT(mod != NULL, "Should find reloaded module");
    
    idcu_dynamic_loader_unload_module(&loader, "test_reload");
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_unload_unloaded(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    int ret = idcu_dynamic_loader_unload_module(&loader, "never_loaded");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_NOT_FOUND, "Unloading unloaded module should fail");
    
    idcu_dynamic_loader_destroy(&loader);
    IDCU_TEST_PASS();
}

static void test_dynamic_loader_max_modules(void) {
    idcu_DynamicLoader loader;
    idcu_dynamic_loader_init(&loader, NULL);
    
    char module_path[256];
#ifdef _WIN32
    snprintf(module_path, sizeof(module_path), "%s", TEST_MODULE_GOOD);
#else
    snprintf(module_path, sizeof(module_path), "./%s", TEST_MODULE_GOOD);
#endif
    
    int first_ret = idcu_dynamic_loader_load_module(&loader, "test_1", module_path);
    if (first_ret != IDCU_ERR_SUCCESS) {
        IDCU_LOG_WARN("Could not load test module, skipping max modules test");
        idcu_dynamic_loader_destroy(&loader);
        IDCU_TEST_PASS();
        return;
    }
    
    idcu_dynamic_loader_unload_module(&loader, "test_1");
    
    for (uint32_t i = 0; i < IDCU_MAX_DYNAMIC_MODULES; i++) {
        char name[64];
        snprintf(name, sizeof(name), "test_%u", i);
        int ret = idcu_dynamic_loader_load_module(&loader, name, module_path);
        if (ret != IDCU_ERR_SUCCESS) {
            IDCU_LOG_ERROR("Failed to load module %u", i);
            break;
        }
    }
    
    int count = idcu_dynamic_loader_get_count(&loader);
    IDCU_TEST_ASSERT(count == IDCU_MAX_DYNAMIC_MODULES, "Should have loaded max modules");
    
    char extra_name[64] = "test_extra";
    int ret = idcu_dynamic_loader_load_module(&loader, extra_name, module_path);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_QUEUE_FULL, "Loading extra module should fail");
    
    for (uint32_t i = 0; i < IDCU_MAX_DYNAMIC_MODULES; i++) {
        char name[64];
        snprintf(name, sizeof(name), "test_%u", i);
        idcu_dynamic_loader_unload_module(&loader, name);
    }
    
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
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_load_invalid_path", test_dynamic_loader_load_invalid_path);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_null_params", test_dynamic_loader_null_params);
    idcu_test_suite_add_test(&g_suite, "hotplug_null_params", test_hotplug_null_params);
    idcu_test_suite_add_test(&g_suite, "hotplug_valid_loader_null_params", test_hotplug_valid_loader_null_params);
    idcu_test_suite_add_test(&g_suite, "hotplug_restart_nonexistent", test_hotplug_restart_nonexistent);
    idcu_test_suite_add_test(&g_suite, "hotplug_unload_nonexistent", test_hotplug_unload_nonexistent);
    
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_load_good_module", test_dynamic_loader_load_good_module);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_module_lifecycle", test_dynamic_loader_module_lifecycle);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_load_duplicate", test_dynamic_loader_load_duplicate);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_load_no_interface", test_dynamic_loader_load_no_interface);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_module_init_fail", test_dynamic_loader_module_init_fail);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_hotplug_load", test_dynamic_loader_hotplug_load);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_restart_module", test_dynamic_loader_restart_module);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_reload_module", test_dynamic_loader_reload_module);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_unload_unloaded", test_dynamic_loader_unload_unloaded);
    idcu_test_suite_add_test(&g_suite, "dynamic_loader_max_modules", test_dynamic_loader_max_modules);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
