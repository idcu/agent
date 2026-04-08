#include "idcu/common/error_code.h"
#include "idcu/sandbox/sandbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(cond, msg)                                                                     \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "FAIL: %s\n", msg);                                                    \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

static int test_sandbox_init_destroy(void) {
    idcu_Sandbox sb;
    int ret = idcu_sandbox_init(&sb, 1, IDCU_PERM_FILE);
    TEST_ASSERT(ret == IDCU_ERR_OK, "idcu_sandbox_init should succeed");

    idcu_sandbox_destroy(&sb);
    printf("PASS: test_sandbox_init_destroy\n");
    return 0;
}

static int test_sandbox_permissions(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);

    // Add permission
    int ret = idcu_sandbox_add_perm(&sb, IDCU_PERM_FILE);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Add perm should succeed");

    // Check permission
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_FILE);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Should have file permission");

    // Check another permission
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_NETWORK);
    TEST_ASSERT(ret != IDCU_ERR_OK, "Should not have network permission");

    // Remove permission
    ret = idcu_sandbox_remove_perm(&sb, IDCU_PERM_FILE);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Remove perm should succeed");

    // Check permission again
    ret = idcu_sandbox_perm_check(&sb, IDCU_PERM_FILE);
    TEST_ASSERT(ret != IDCU_ERR_OK, "Should not have file permission after remove");

    // Set all permissions
    ret = idcu_sandbox_set_perm(&sb, IDCU_PERM_ALL);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Set perm should succeed");

    // Get permissions
    uint32_t perm = idcu_sandbox_get_perm(&sb);
    TEST_ASSERT(perm == IDCU_PERM_ALL, "Should have all permissions");

    idcu_sandbox_destroy(&sb);
    printf("PASS: test_sandbox_permissions\n");
    return 0;
}

static int test_sandbox_quota(void) {
    idcu_Sandbox sb;
    idcu_sandbox_init(&sb, 1, 0);

    // Set quota
    int ret = idcu_sandbox_set_quota(&sb, 1024);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Set quota should succeed");

    // Get quota
    uint32_t quota = idcu_sandbox_get_quota(&sb);
    TEST_ASSERT(quota == 1024, "Quota should be 1024");

    idcu_sandbox_destroy(&sb);
    printf("PASS: test_sandbox_quota\n");
    return 0;
}

static int test_sandbox_registry(void) {
    idcu_Sandbox sb1, sb2;
    idcu_sandbox_init(&sb1, 1, IDCU_PERM_FILE);
    idcu_sandbox_init(&sb2, 2, IDCU_PERM_NETWORK);

    // Init registry
    int ret = idcu_sandbox_registry_init();
    TEST_ASSERT(ret == IDCU_ERR_OK, "Registry init should succeed");

    // Add sandbox
    ret = idcu_sandbox_registry_add(&sb1);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Add sandbox 1 should succeed");

    ret = idcu_sandbox_registry_add(&sb2);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Add sandbox 2 should succeed");

    // Get sandbox
    idcu_Sandbox *sb_ptr = idcu_sandbox_registry_get(1);
    TEST_ASSERT(sb_ptr != NULL, "Should get sandbox 1");

    // Check permission
    ret = idcu_sandbox_registry_check_perm(1, IDCU_PERM_FILE);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Sandbox 1 should have file permission");

    ret = idcu_sandbox_registry_check_perm(2, IDCU_PERM_NETWORK);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Sandbox 2 should have network permission");

    // Remove sandbox
    ret = idcu_sandbox_registry_remove(1);
    TEST_ASSERT(ret == IDCU_ERR_OK, "Remove sandbox 1 should succeed");

    sb_ptr = idcu_sandbox_registry_get(1);
    TEST_ASSERT(sb_ptr == NULL, "Sandbox 1 should be removed");

    // Destroy registry
    idcu_sandbox_registry_destroy();

    idcu_sandbox_destroy(&sb1);
    idcu_sandbox_destroy(&sb2);
    printf("PASS: test_sandbox_registry\n");
    return 0;
}

int main(void) {
    int failures = 0;

    if (test_sandbox_init_destroy() != 0)
        failures++;
    if (test_sandbox_permissions() != 0)
        failures++;
    if (test_sandbox_quota() != 0)
        failures++;
    if (test_sandbox_registry() != 0)
        failures++;

    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
