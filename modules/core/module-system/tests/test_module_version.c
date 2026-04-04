#include "test_framework.h"
#include "micro_kernel.h"
#include "module_version.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

static idcu_MicroKernel g_kernel;

idcu_MicroKernel* idcu_get_kernel(void)
{
    return &g_kernel;
}

static idcu_TestSuite g_suite;

static void test_version_create(void) {
    idcu_ModuleVersion v1 = idcu_version_create(1, 2, 3);
    IDCU_TEST_ASSERT(v1.major == 1, "major version should be 1");
    IDCU_TEST_ASSERT(v1.minor == 2, "minor version should be 2");
    IDCU_TEST_ASSERT(v1.patch == 3, "patch version should be 3");
    IDCU_TEST_ASSERT(v1.pre_release == NULL, "pre_release should be NULL");
    
    idcu_ModuleVersion v2 = idcu_version_create_pre(2, 0, 0, "alpha");
    IDCU_TEST_ASSERT(v2.major == 2, "major version should be 2");
    IDCU_TEST_ASSERT(v2.minor == 0, "minor version should be 0");
    IDCU_TEST_ASSERT(v2.patch == 0, "patch version should be 0");
    IDCU_TEST_ASSERT(v2.pre_release != NULL, "pre_release should not be NULL");
    IDCU_TEST_ASSERT(strcmp(v2.pre_release, "alpha") == 0, "pre_release should be 'alpha'");
    IDCU_TEST_PASS();
}

static void test_version_format(void) {
    char buffer[IDCU_VERSION_STR_MAX];
    idcu_ModuleVersion v = idcu_version_create(1, 2, 3);
    
    int result = idcu_version_format(&v, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(result == IDCU_SUCCESS, "version format should succeed");
    IDCU_TEST_ASSERT(strcmp(buffer, "1.2.3") == 0, "formatted version should be '1.2.3'");
    
    idcu_ModuleVersion v_pre = idcu_version_create_pre(2, 0, 0, "beta.1");
    result = idcu_version_format(&v_pre, buffer, sizeof(buffer));
    IDCU_TEST_ASSERT(result == IDCU_SUCCESS, "version format with pre-release should succeed");
    IDCU_TEST_ASSERT(strcmp(buffer, "2.0.0-beta.1") == 0, "formatted version should be '2.0.0-beta.1'");
    IDCU_TEST_PASS();
}

static void test_version_parse(void) {
    idcu_ModuleVersion v;
    int result = idcu_version_parse("1.2.3", &v);
    IDCU_TEST_ASSERT(result == IDCU_SUCCESS, "version parse should succeed");
    IDCU_TEST_ASSERT(v.major == 1, "major version should be 1");
    IDCU_TEST_ASSERT(v.minor == 2, "minor version should be 2");
    IDCU_TEST_ASSERT(v.patch == 3, "patch version should be 3");
    IDCU_TEST_ASSERT(v.pre_release == NULL, "pre_release should be NULL");
    
    result = idcu_version_parse("2.0.0-alpha", &v);
    IDCU_TEST_ASSERT(result == IDCU_SUCCESS, "version parse with pre-release should succeed");
    IDCU_TEST_ASSERT(v.major == 2, "major version should be 2");
    IDCU_TEST_ASSERT(v.minor == 0, "minor version should be 0");
    IDCU_TEST_ASSERT(v.patch == 0, "patch version should be 0");
    IDCU_TEST_ASSERT(v.pre_release != NULL, "pre_release should not be NULL");
    IDCU_TEST_ASSERT(strcmp(v.pre_release, "alpha") == 0, "pre_release should be 'alpha'");
    IDCU_TEST_PASS();
}

static void test_version_compare(void) {
    idcu_ModuleVersion v1 = idcu_version_create(1, 0, 0);
    idcu_ModuleVersion v2 = idcu_version_create(1, 0, 0);
    idcu_ModuleVersion v3 = idcu_version_create(1, 1, 0);
    idcu_ModuleVersion v4 = idcu_version_create(2, 0, 0);
    idcu_ModuleVersion v5 = idcu_version_create_pre(1, 0, 0, "alpha");
    idcu_ModuleVersion v6 = idcu_version_create_pre(1, 0, 0, "beta");
    
    IDCU_TEST_ASSERT(idcu_version_compare(&v1, &v2) == IDCU_VERSION_EQUAL, "versions should be equal");
    IDCU_TEST_ASSERT(idcu_version_compare(&v1, &v3) == IDCU_VERSION_LESS, "v1 should be less than v3");
    IDCU_TEST_ASSERT(idcu_version_compare(&v3, &v1) == IDCU_VERSION_GREATER, "v3 should be greater than v1");
    IDCU_TEST_ASSERT(idcu_version_compare(&v1, &v4) == IDCU_VERSION_LESS, "v1 should be less than v4");
    IDCU_TEST_ASSERT(idcu_version_compare(&v5, &v1) == IDCU_VERSION_LESS, "v5 should be less than v1");
    IDCU_TEST_ASSERT(idcu_version_compare(&v5, &v6) == IDCU_VERSION_LESS, "v5 should be less than v6");
    IDCU_TEST_PASS();
}

static void test_version_compatibility(void) {
    idcu_ModuleVersion v1 = idcu_version_create(1, 0, 0);
    idcu_ModuleVersion v2 = idcu_version_create(1, 5, 3);
    idcu_ModuleVersion v3 = idcu_version_create(2, 0, 0);
    
    IDCU_TEST_ASSERT(idcu_version_is_compatible(&v2, &v1) != 0, "v2 should be compatible with v1");
    IDCU_TEST_ASSERT(idcu_version_is_compatible(&v3, &v1) == 0, "v3 should not be compatible with v1");
    IDCU_TEST_PASS();
}

static void test_version_check_dependency(void) {
    idcu_ModuleVersion actual = idcu_version_create(1, 5, 0);
    
    idcu_ModuleDependency dep1 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(1, 0, 0),
        .has_max_version = 0
    };
    IDCU_TEST_ASSERT(idcu_version_check_dependency(&actual, &dep1) == IDCU_SUCCESS, "dep1 should be satisfied");
    
    idcu_ModuleDependency dep2 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(2, 0, 0),
        .has_max_version = 0
    };
    IDCU_TEST_ASSERT(idcu_version_check_dependency(&actual, &dep2) == IDCU_ERROR_VERSION_TOO_OLD, "dep2 should fail with too old");
    
    idcu_ModuleDependency dep3 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(1, 0, 0),
        .max_version = idcu_version_create(1, 4, 0),
        .has_max_version = 1
    };
    IDCU_TEST_ASSERT(idcu_version_check_dependency(&actual, &dep3) == IDCU_ERROR_VERSION_TOO_NEW, "dep3 should fail with too new");
    
    idcu_ModuleDependency dep4 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(2, 0, 0),
        .has_max_version = 0
    };
    idcu_ModuleVersion actual_v2 = idcu_version_create(2, 0, 0);
    IDCU_TEST_ASSERT(idcu_version_check_dependency(&actual_v2, &dep4) == IDCU_SUCCESS, "dep4 should be satisfied for v2");
    IDCU_TEST_PASS();
}

int main(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);
    
    idcu_test_suite_init(&g_suite, "Module Version Tests");
    
    idcu_test_suite_add_test(&g_suite, "version_create", test_version_create);
    idcu_test_suite_add_test(&g_suite, "version_format", test_version_format);
    idcu_test_suite_add_test(&g_suite, "version_parse", test_version_parse);
    idcu_test_suite_add_test(&g_suite, "version_compare", test_version_compare);
    idcu_test_suite_add_test(&g_suite, "version_compatibility", test_version_compatibility);
    idcu_test_suite_add_test(&g_suite, "version_check_dependency", test_version_check_dependency);
    
    idcu_test_suite_run(&g_suite);
    idcu_test_suite_print_summary(&g_suite);
    
    int failures = idcu_test_suite_get_failures(&g_suite);
    
    return failures > 0 ? 1 : 0;
}
