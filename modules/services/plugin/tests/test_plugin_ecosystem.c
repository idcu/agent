#include "../include/plugin_ecosystem.h"
#include "idcu/log/log.h"
#include <stdio.h>
#include <string.h>

static void test_plugin_ecosystem_init_destroy(void) {
    idcu_PluginEcosystem eco;
    int ret = idcu_plugin_ecosystem_init(&eco);
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_ecosystem_init_destroy: FAIL - idcu_plugin_ecosystem_init should "
               "succeed\n");
        return;
    }

    idcu_plugin_ecosystem_destroy(&eco);
    printf("test_plugin_ecosystem_init_destroy: PASS\n");
}

static void test_plugin_add_remove(void) {
    idcu_PluginEcosystem eco;
    idcu_plugin_ecosystem_init(&eco);

    int ret = idcu_plugin_ecosystem_add_plugin(&eco, "test-plugin", "1.0.0", "A test plugin");
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_add_remove: FAIL - Add plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    ret = idcu_plugin_ecosystem_add_plugin(&eco, "another-plugin", "2.1.3", "Another test plugin");
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_add_remove: FAIL - Add another plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    idcu_PluginInfo *info = idcu_plugin_ecosystem_find_plugin(&eco, "test-plugin");
    if (info == NULL) {
        printf("test_plugin_add_remove: FAIL - Find plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    if (strcmp(info->name, "test-plugin") != 0) {
        printf("test_plugin_add_remove: FAIL - Plugin name should match\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    ret = idcu_plugin_ecosystem_remove_plugin(&eco, "test-plugin");
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_add_remove: FAIL - Remove plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    info = idcu_plugin_ecosystem_find_plugin(&eco, "test-plugin");
    if (info != NULL) {
        printf("test_plugin_add_remove: FAIL - Find removed plugin should return NULL\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    idcu_plugin_ecosystem_destroy(&eco);
    printf("test_plugin_add_remove: PASS\n");
}

static void test_plugin_lifecycle(void) {
    idcu_PluginEcosystem eco;
    idcu_plugin_ecosystem_init(&eco);

    idcu_plugin_ecosystem_add_plugin(&eco, "lifecycle-test", "1.0.0", "Lifecycle test plugin");

    int ret = idcu_plugin_ecosystem_install_plugin(&eco, "lifecycle-test");
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_lifecycle: FAIL - Install plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    idcu_PluginInfo *info = idcu_plugin_ecosystem_find_plugin(&eco, "lifecycle-test");
    if (info->status != IDCU_PLUGIN_STATUS_INSTALLED) {
        printf("test_plugin_lifecycle: FAIL - Status should be INSTALLED\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    ret = idcu_plugin_ecosystem_activate_plugin(&eco, "lifecycle-test");
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_lifecycle: FAIL - Activate plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }
    if (info->status != IDCU_PLUGIN_STATUS_ACTIVE) {
        printf("test_plugin_lifecycle: FAIL - Status should be ACTIVE\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    ret = idcu_plugin_ecosystem_deactivate_plugin(&eco, "lifecycle-test");
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_lifecycle: FAIL - Deactivate plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }
    if (info->status != IDCU_PLUGIN_STATUS_INSTALLED) {
        printf("test_plugin_lifecycle: FAIL - Status should be INSTALLED\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    ret = idcu_plugin_ecosystem_uninstall_plugin(&eco, "lifecycle-test");
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_lifecycle: FAIL - Uninstall plugin should succeed\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }
    if (info->status != IDCU_PLUGIN_STATUS_AVAILABLE) {
        printf("test_plugin_lifecycle: FAIL - Status should be AVAILABLE\n");
        idcu_plugin_ecosystem_destroy(&eco);
        return;
    }

    idcu_plugin_ecosystem_destroy(&eco);
    printf("test_plugin_lifecycle: PASS\n");
}

static void test_plugin_version_parsing(void) {
    uint32_t major, minor, patch;

    int ret = idcu_plugin_version_parse("1.0.0", &major, &minor, &patch);
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_version_parsing: FAIL - Parse 1.0.0 should succeed\n");
        return;
    }
    if (major != 1 || minor != 0 || patch != 0) {
        printf("test_plugin_version_parsing: FAIL - Version should be 1.0.0\n");
        return;
    }

    ret = idcu_plugin_version_parse("2.5.3", &major, &minor, &patch);
    if (ret != IDCU_ERR_OK) {
        printf("test_plugin_version_parsing: FAIL - Parse 2.5.3 should succeed\n");
        return;
    }
    if (major != 2 || minor != 5 || patch != 3) {
        printf("test_plugin_version_parsing: FAIL - Version should be 2.5.3\n");
        return;
    }

    printf("test_plugin_version_parsing: PASS\n");
}

static void test_plugin_version_compare(void) {
    int ret = idcu_plugin_version_compare(1, 0, 0, 1, 0, 0);
    if (ret != 0) {
        printf("test_plugin_version_compare: FAIL - Same versions should be equal\n");
        return;
    }

    ret = idcu_plugin_version_compare(1, 0, 0, 2, 0, 0);
    if (ret >= 0) {
        printf("test_plugin_version_compare: FAIL - 1.0.0 should be less than 2.0.0\n");
        return;
    }

    ret = idcu_plugin_version_compare(2, 0, 0, 1, 0, 0);
    if (ret <= 0) {
        printf("test_plugin_version_compare: FAIL - 2.0.0 should be greater than 1.0.0\n");
        return;
    }

    ret = idcu_plugin_version_compare(1, 1, 0, 1, 0, 0);
    if (ret <= 0) {
        printf("test_plugin_version_compare: FAIL - 1.1.0 should be greater than 1.0.0\n");
        return;
    }

    ret = idcu_plugin_version_compare(1, 0, 5, 1, 0, 3);
    if (ret <= 0) {
        printf("test_plugin_version_compare: FAIL - 1.0.5 should be greater than 1.0.3\n");
        return;
    }

    printf("test_plugin_version_compare: PASS\n");
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    printf("=== Plugin Ecosystem Tests ===\n");

    test_plugin_ecosystem_init_destroy();
    test_plugin_add_remove();
    test_plugin_lifecycle();
    test_plugin_version_parsing();
    test_plugin_version_compare();

    printf("=== All Tests Completed ===\n");

    return 0;
}
