#include "idcu/common/error_code.h"
#include "idcu/permission/permission.h"
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

static int test_permission_init_shutdown(void) {
    int ret = idcu_permission_manager_init();
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "idcu_permission_manager_init should succeed");

    idcu_permission_manager_shutdown();
    printf("PASS: test_permission_init_shutdown\n");
    return 0;
}

static int test_permission_grant_check(void) {
    idcu_permission_manager_init();

    int ret = idcu_permission_grant("module1", "file.read");
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Grant should succeed");

    int has_perm = idcu_permission_check("module1", "file.read");
    TEST_ASSERT(has_perm, "Should have permission");

    has_perm = idcu_permission_check("module1", "file.write");
    TEST_ASSERT(!has_perm, "Should not have permission");

    idcu_permission_manager_shutdown();
    printf("PASS: test_permission_grant_check\n");
    return 0;
}

static int test_permission_revoke(void) {
    idcu_permission_manager_init();

    idcu_permission_grant("module1", "file.read");
    idcu_permission_grant("module1", "file.write");

    int has_perm = idcu_permission_check("module1", "file.read");
    TEST_ASSERT(has_perm, "Should have permission");

    int ret = idcu_permission_revoke("module1", "file.read");
    TEST_ASSERT(ret == IDCU_ERR_SUCCESS, "Revoke should succeed");

    has_perm = idcu_permission_check("module1", "file.read");
    TEST_ASSERT(!has_perm, "Should not have permission after revoke");

    has_perm = idcu_permission_check("module1", "file.write");
    TEST_ASSERT(has_perm, "Should still have other permission");

    idcu_permission_manager_shutdown();
    printf("PASS: test_permission_revoke\n");
    return 0;
}

static int test_permission_check_any_all(void) {
    idcu_permission_manager_init();

    idcu_permission_grant("module1", "file.read");
    idcu_permission_grant("module1", "network.connect");

    const char *perms1[] = {"file.read", "file.write"};
    int has_any = idcu_permission_check_any("module1", perms1, 2);
    TEST_ASSERT(has_any, "Should have any permission");

    const char *perms2[] = {"file.read", "network.connect"};
    int has_all = idcu_permission_check_all("module1", perms2, 2);
    TEST_ASSERT(has_all, "Should have all permissions");

    const char *perms3[] = {"file.read", "file.write", "network.listen"};
    has_all = idcu_permission_check_all("module1", perms3, 3);
    TEST_ASSERT(!has_all, "Should not have all permissions");

    idcu_permission_manager_shutdown();
    printf("PASS: test_permission_check_any_all\n");
    return 0;
}

int main(void) {
    int failures = 0;

    if (test_permission_init_shutdown() != 0)
        failures++;
    if (test_permission_grant_check() != 0)
        failures++;
    if (test_permission_revoke() != 0)
        failures++;
    if (test_permission_check_any_all() != 0)
        failures++;

    if (failures == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("%d tests failed!\n", failures);
        return 1;
    }
}
