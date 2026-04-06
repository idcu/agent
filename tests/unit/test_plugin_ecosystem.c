#include "test/test_framework.h"
#include "plugin_ecosystem.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static idcu_TestSuite g_suite;

static void test_plugin_ecosystem_init_destroy(void) {
    idcu_PluginEcosystem eco;
    int ret = idcu_plugin_ecosystem_init(&eco);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_plugin_ecosystem_init should succeed");
    
    idcu_plugin_ecosystem_destroy(&eco);
    IDCU_TEST_PASS();
}

static void test_plugin_add_remove(void) {
    idcu_PluginEcosystem eco;
    idcu_plugin_ecosystem_init(&eco);
    
    int ret = idcu_plugin_ecosystem_add_plugin(&eco, "test-plugin", "1.0.0", "A test plugin");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add plugin should succeed");
    
    ret = idcu_plugin_ecosystem_add_plugin(&eco, "another-plugin", "2.1.3", "Another test plugin");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Add another plugin should succeed");
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(&eco, "test-plugin");
    IDCU_TEST_ASSERT(info != NULL, "Find plugin should succeed");
    IDCU_TEST_ASSERT(strcmp(info->name, "test-plugin") == 0, "Plugin name should match");
    
    ret = idcu_plugin_ecosystem_remove_plugin(&eco, "test-plugin");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Remove plugin should succeed");
    
    info = idcu_plugin_ecosystem_find_plugin(&eco, "test-plugin");
    IDCU_TEST_ASSERT(info == NULL, "Find removed plugin should return NULL");
    
    idcu_plugin_ecosystem_destroy(&eco);
    IDCU_TEST_PASS();
}

static void test_plugin_lifecycle(void) {
    idcu_PluginEcosystem eco;
    idcu_plugin_ecosystem_init(&eco);
    
    idcu_plugin_ecosystem_add_plugin(&eco, "lifecycle-test", "1.0.0", "Lifecycle test plugin");
    
    int ret = idcu_plugin_ecosystem_install_plugin(&eco, "lifecycle-test");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Install plugin should succeed");
    
    idcu_PluginInfo* info = idcu_plugin_ecosystem_find_plugin(&eco, "lifecycle-test");
    IDCU_TEST_ASSERT(info->status == IDCU_PLUGIN_STATUS_INSTALLED, "Status should be INSTALLED");
    
    ret = idcu_plugin_ecosystem_activate_plugin(&eco, "lifecycle-test");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Activate plugin should succeed");
    IDCU_TEST_ASSERT(info->status == IDCU_PLUGIN_STATUS_ACTIVE, "Status should be ACTIVE");
    
    ret = idcu_plugin_ecosystem_deactivate_plugin(&eco, "lifecycle-test");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Deactivate plugin should succeed");
    IDCU_TEST_ASSERT(info->status == IDCU_PLUGIN_STATUS_INSTALLED, "Status should be INSTALLED");
    
    ret = idcu_plugin_ecosystem_uninstall_plugin(&eco, "lifecycle-test");
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Uninstall plugin should succeed");
    IDCU_TEST_ASSERT(info->status == IDCU_PLUGIN_STATUS_AVAILABLE, "Status should be AVAILABLE");
    
    idcu_plugin_ecosystem_destroy(&eco);
    IDCU_TEST_PASS();
}

static void test_plugin_version_parsing(void) {
    uint32_t major, minor, patch;
    
    int ret = idcu_plugin_version_parse("1.0.0", &major, &minor, &patch);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Parse 1.0.0 should succeed");
    IDCU_TEST_ASSERT(major == 1 && minor == 0 && patch == 0, "Version should be 1.0.0");
    
    ret = idcu_plugin_version_parse("2.5.3", &major, &minor, &patch);
    IDCU_TEST_ASSERT(ret == IDCU_ERR_OK, "Parse 2.5.3 should succeed");
    IDCU_TEST_ASSERT(major == 2 && minor == 5 && patch == 3, "Version should be 2.5.3");
    
    IDCU_TEST_PASS();
}

static void test_plugin_version_compare(void) {
    int ret = idcu_plugin_version_compare(1, 0, 0, 1, 0, 0);
    IDCU_TEST_ASSERT(ret == 0, "Same versions should be equal");
    
    ret = idcu_plugin_version_compare(1, 0, 0, 2, 0, 0);
    IDCU_TEST_ASSERT(ret < 0, "1.0.0 should be less than 2.0.0");
    
    ret = idcu_plugin_version_compare(2, 0, 0, 1, 0, 0);
    IDCU_TEST_ASSERT(ret > 0, "2.0.0 should be greater than 1.0.0");
    
    ret = idcu_plugin_version_compare(1, 1, 0, 1, 0, 0);
    IDCU_TEST_ASSERT(ret > 0, "1.1.0 should be greater than 1.0.0");
    
    ret = idcu_plugin_version_compare(1, 0, 5, 1, 0, 3);
    IDCU_TEST_ASSERT(ret > 0, "1.0.5 should be greater than 1.0.3");
    
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Plugin Ecosystem Tests");
    
    idcu_test_suite_add_test(&g_suite, "plugin_ecosystem_init_destroy", test_plugin_ecosystem_init_destroy);
    idcu_test_suite_add_test(&g_suite, "plugin_add_remove", test_plugin_add_remove);
    idcu_test_suite_add_test(&g_suite, "plugin_lifecycle", test_plugin_lifecycle);
    idcu_test_suite_add_test(&g_suite, "plugin_version_parsing", test_plugin_version_parsing);
    idcu_test_suite_add_test(&g_suite, "plugin_version_compare", test_plugin_version_compare);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
