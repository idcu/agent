#include "test/test_framework.h"
#include "plugin/plugin_ecosystem.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static TestSuite g_suite;

static void test_plugin_ecosystem_init_destroy(void) {
    PluginEcosystem eco;
    int ret = plugin_ecosystem_init(&eco);
    TEST_ASSERT(ret == ERR_OK, "plugin_ecosystem_init should succeed");
    TEST_ASSERT(eco.plugin_count == 0, "Plugin count should be 0");
    
    plugin_ecosystem_destroy(&eco);
    TEST_PASS();
}

static void test_plugin_add_remove(void) {
    PluginEcosystem eco;
    plugin_ecosystem_init(&eco);
    
    int ret = plugin_ecosystem_add_plugin(&eco, "test-plugin", "1.0.0", "A test plugin");
    TEST_ASSERT(ret == ERR_OK, "Add plugin should succeed");
    TEST_ASSERT(eco.plugin_count == 1, "Plugin count should be 1");
    
    ret = plugin_ecosystem_add_plugin(&eco, "another-plugin", "2.1.3", "Another test plugin");
    TEST_ASSERT(ret == ERR_OK, "Add another plugin should succeed");
    TEST_ASSERT(eco.plugin_count == 2, "Plugin count should be 2");
    
    PluginInfo* info = plugin_ecosystem_find_plugin(&eco, "test-plugin");
    TEST_ASSERT(info != NULL, "Find plugin should succeed");
    TEST_ASSERT(strcmp(info->name, "test-plugin") == 0, "Plugin name should match");
    TEST_ASSERT(info->major == 1 && info->minor == 0 && info->patch == 0, "Version should be 1.0.0");
    TEST_ASSERT(info->status == PLUGIN_STATUS_AVAILABLE, "Status should be AVAILABLE");
    
    ret = plugin_ecosystem_remove_plugin(&eco, "test-plugin");
    TEST_ASSERT(ret == ERR_OK, "Remove plugin should succeed");
    TEST_ASSERT(eco.plugin_count == 1, "Plugin count should be 1");
    
    info = plugin_ecosystem_find_plugin(&eco, "test-plugin");
    TEST_ASSERT(info == NULL, "Find removed plugin should return NULL");
    
    plugin_ecosystem_destroy(&eco);
    TEST_PASS();
}

static void test_plugin_lifecycle(void) {
    PluginEcosystem eco;
    plugin_ecosystem_init(&eco);
    
    plugin_ecosystem_add_plugin(&eco, "lifecycle-test", "1.0.0", "Lifecycle test plugin");
    
    int ret = plugin_ecosystem_install_plugin(&eco, "lifecycle-test");
    TEST_ASSERT(ret == ERR_OK, "Install plugin should succeed");
    
    PluginInfo* info = plugin_ecosystem_find_plugin(&eco, "lifecycle-test");
    TEST_ASSERT(info->status == PLUGIN_STATUS_INSTALLED, "Status should be INSTALLED");
    
    ret = plugin_ecosystem_activate_plugin(&eco, "lifecycle-test");
    TEST_ASSERT(ret == ERR_OK, "Activate plugin should succeed");
    TEST_ASSERT(info->status == PLUGIN_STATUS_ACTIVE, "Status should be ACTIVE");
    
    ret = plugin_ecosystem_deactivate_plugin(&eco, "lifecycle-test");
    TEST_ASSERT(ret == ERR_OK, "Deactivate plugin should succeed");
    TEST_ASSERT(info->status == PLUGIN_STATUS_INSTALLED, "Status should be INSTALLED");
    
    ret = plugin_ecosystem_uninstall_plugin(&eco, "lifecycle-test");
    TEST_ASSERT(ret == ERR_OK, "Uninstall plugin should succeed");
    TEST_ASSERT(info->status == PLUGIN_STATUS_AVAILABLE, "Status should be AVAILABLE");
    
    plugin_ecosystem_destroy(&eco);
    TEST_PASS();
}

static void test_plugin_version_parsing(void) {
    uint32_t major, minor, patch;
    
    int ret = plugin_version_parse("1.0.0", &major, &minor, &patch);
    TEST_ASSERT(ret == ERR_OK, "Parse 1.0.0 should succeed");
    TEST_ASSERT(major == 1 && minor == 0 && patch == 0, "Version should be 1.0.0");
    
    ret = plugin_version_parse("2.5.3", &major, &minor, &patch);
    TEST_ASSERT(ret == ERR_OK, "Parse 2.5.3 should succeed");
    TEST_ASSERT(major == 2 && minor == 5 && patch == 3, "Version should be 2.5.3");
    
    ret = plugin_version_parse("invalid", &major, &minor, &patch);
    TEST_ASSERT(ret != ERR_OK, "Parse invalid should fail");
    
    plugin_ecosystem_destroy(NULL);
    TEST_PASS();
}

static void test_plugin_version_compare(void) {
    int ret = plugin_version_compare(1, 0, 0, 1, 0, 0);
    TEST_ASSERT(ret == 0, "Same versions should be equal");
    
    ret = plugin_version_compare(1, 0, 0, 2, 0, 0);
    TEST_ASSERT(ret < 0, "1.0.0 should be less than 2.0.0");
    
    ret = plugin_version_compare(2, 0, 0, 1, 0, 0);
    TEST_ASSERT(ret > 0, "2.0.0 should be greater than 1.0.0");
    
    ret = plugin_version_compare(1, 1, 0, 1, 0, 0);
    TEST_ASSERT(ret > 0, "1.1.0 should be greater than 1.0.0");
    
    ret = plugin_version_compare(1, 0, 5, 1, 0, 3);
    TEST_ASSERT(ret > 0, "1.0.5 should be greater than 1.0.3");
    
    TEST_PASS();
}

static void test_plugin_rating(void) {
    PluginEcosystem eco;
    plugin_ecosystem_init(&eco);
    
    plugin_ecosystem_add_plugin(&eco, "rating-test", "1.0.0", "Rating test plugin");
    
    int ret = plugin_ecosystem_rate_plugin(&eco, "rating-test", 5.0f);
    TEST_ASSERT(ret == ERR_OK, "Rate plugin should succeed");
    
    PluginInfo* info = plugin_ecosystem_find_plugin(&eco, "rating-test");
    TEST_ASSERT(info->rating == 5.0f, "Rating should be 5.0");
    TEST_ASSERT(info->rating_count == 1, "Rating count should be 1");
    
    ret = plugin_ecosystem_rate_plugin(&eco, "rating-test", 3.0f);
    TEST_ASSERT(ret == ERR_OK, "Rate plugin again should succeed");
    TEST_ASSERT(info->rating == 4.0f, "Average rating should be 4.0");
    TEST_ASSERT(info->rating_count == 2, "Rating count should be 2");
    
    plugin_ecosystem_destroy(&eco);
    TEST_PASS();
}

static void test_plugin_dependencies(void) {
    PluginEcosystem eco;
    plugin_ecosystem_init(&eco);
    
    plugin_ecosystem_add_plugin(&eco, "dep-a", "1.0.0", "Dependency A");
    plugin_ecosystem_add_plugin(&eco, "main-plugin", "1.0.0", "Main plugin with dependencies");
    
    PluginDependency deps[1];
    strcpy(deps[0].name, "dep-a");
    strcpy(deps[0].version, "1.0.0");
    deps[0].major = 1;
    deps[0].minor = 0;
    deps[0].patch = 0;
    
    int ret = plugin_ecosystem_set_dependencies(&eco, "main-plugin", deps, 1);
    TEST_ASSERT(ret == ERR_OK, "Set dependencies should succeed");
    
    PluginInfo* info = plugin_ecosystem_find_plugin(&eco, "main-plugin");
    TEST_ASSERT(info->dep_count == 1, "Dependency count should be 1");
    TEST_ASSERT(strcmp(info->dependencies[0].name, "dep-a") == 0, "Dependency name should be dep-a");
    
    ret = plugin_ecosystem_check_compatibility(&eco, "dep-a", "1.0.0");
    TEST_ASSERT(ret == ERR_OK, "Compatibility check for 1.0.0 should succeed");
    
    ret = plugin_ecosystem_check_compatibility(&eco, "dep-a", "2.0.0");
    TEST_ASSERT(ret != ERR_OK, "Compatibility check for 2.0.0 should fail");
    
    plugin_ecosystem_destroy(&eco);
    TEST_PASS();
}

int main(void) {
    log_init(NULL, LOG_INFO);
    
    test_suite_init(&g_suite, "Plugin Ecosystem Tests");
    
    test_suite_add_test(&g_suite, "plugin_ecosystem_init_destroy", test_plugin_ecosystem_init_destroy);
    test_suite_add_test(&g_suite, "plugin_add_remove", test_plugin_add_remove);
    test_suite_add_test(&g_suite, "plugin_lifecycle", test_plugin_lifecycle);
    test_suite_add_test(&g_suite, "plugin_version_parsing", test_plugin_version_parsing);
    test_suite_add_test(&g_suite, "plugin_version_compare", test_plugin_version_compare);
    test_suite_add_test(&g_suite, "plugin_rating", test_plugin_rating);
    test_suite_add_test(&g_suite, "plugin_dependencies", test_plugin_dependencies);
    
    test_suite_run(&g_suite);
    test_suite_print_summary(&g_suite);
    
    int failures = test_suite_get_failures(&g_suite);
    log_shutdown();
    
    return failures > 0 ? 1 : 0;
}
