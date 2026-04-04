#include "test/test_framework.h"
#include "module/module_version.h"
#include <string.h>

static void test_version_create(void) {
    idcu_ModuleVersion v1 = idcu_version_create(1, 2, 3);
    TEST_ASSERT_EQUAL(v1.major, 1);
    TEST_ASSERT_EQUAL(v1.minor, 2);
    TEST_ASSERT_EQUAL(v1.patch, 3);
    TEST_ASSERT_NULL(v1.pre_release);
    
    idcu_ModuleVersion v2 = idcu_version_create_pre(2, 0, 0, "alpha");
    TEST_ASSERT_EQUAL(v2.major, 2);
    TEST_ASSERT_EQUAL(v2.minor, 0);
    TEST_ASSERT_EQUAL(v2.patch, 0);
    TEST_ASSERT_NOT_NULL(v2.pre_release);
    TEST_ASSERT_EQUAL_STRING(v2.pre_release, "alpha");
}

static void test_version_format(void) {
    char buffer[IDCU_VERSION_STR_MAX];
    idcu_ModuleVersion v = idcu_version_create(1, 2, 3);
    
    int result = idcu_version_format(&v, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(result, IDCU_SUCCESS);
    TEST_ASSERT_EQUAL_STRING(buffer, "1.2.3");
    
    idcu_ModuleVersion v_pre = idcu_version_create_pre(2, 0, 0, "beta.1");
    result = idcu_version_format(&v_pre, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(result, IDCU_SUCCESS);
    TEST_ASSERT_EQUAL_STRING(buffer, "2.0.0-beta.1");
}

static void test_version_parse(void) {
    idcu_ModuleVersion v;
    int result = idcu_version_parse("1.2.3", &v);
    TEST_ASSERT_EQUAL(result, IDCU_SUCCESS);
    TEST_ASSERT_EQUAL(v.major, 1);
    TEST_ASSERT_EQUAL(v.minor, 2);
    TEST_ASSERT_EQUAL(v.patch, 3);
    TEST_ASSERT_NULL(v.pre_release);
    
    result = idcu_version_parse("2.0.0-alpha", &v);
    TEST_ASSERT_EQUAL(result, IDCU_SUCCESS);
    TEST_ASSERT_EQUAL(v.major, 2);
    TEST_ASSERT_EQUAL(v.minor, 0);
    TEST_ASSERT_EQUAL(v.patch, 0);
    TEST_ASSERT_NOT_NULL(v.pre_release);
    TEST_ASSERT_EQUAL_STRING(v.pre_release, "alpha");
}

static void test_version_compare(void) {
    idcu_ModuleVersion v1 = idcu_version_create(1, 0, 0);
    idcu_ModuleVersion v2 = idcu_version_create(1, 0, 0);
    idcu_ModuleVersion v3 = idcu_version_create(1, 1, 0);
    idcu_ModuleVersion v4 = idcu_version_create(2, 0, 0);
    idcu_ModuleVersion v5 = idcu_version_create_pre(1, 0, 0, "alpha");
    idcu_ModuleVersion v6 = idcu_version_create_pre(1, 0, 0, "beta");
    
    TEST_ASSERT_EQUAL(idcu_version_compare(&v1, &v2), IDCU_VERSION_EQUAL);
    TEST_ASSERT_EQUAL(idcu_version_compare(&v1, &v3), IDCU_VERSION_LESS);
    TEST_ASSERT_EQUAL(idcu_version_compare(&v3, &v1), IDCU_VERSION_GREATER);
    TEST_ASSERT_EQUAL(idcu_version_compare(&v1, &v4), IDCU_VERSION_LESS);
    TEST_ASSERT_EQUAL(idcu_version_compare(&v5, &v1), IDCU_VERSION_LESS);
    TEST_ASSERT_EQUAL(idcu_version_compare(&v5, &v6), IDCU_VERSION_LESS);
}

static void test_version_compatibility(void) {
    idcu_ModuleVersion v1 = idcu_version_create(1, 0, 0);
    idcu_ModuleVersion v2 = idcu_version_create(1, 5, 3);
    idcu_ModuleVersion v3 = idcu_version_create(2, 0, 0);
    
    TEST_ASSERT_TRUE(idcu_version_is_compatible(&v2, &v1));
    TEST_ASSERT_FALSE(idcu_version_is_compatible(&v3, &v1));
}

static void test_version_check_dependency(void) {
    idcu_ModuleVersion actual = idcu_version_create(1, 5, 0);
    
    idcu_ModuleDependency dep1 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(1, 0, 0),
        .has_max_version = 0
    };
    TEST_ASSERT_EQUAL(idcu_version_check_dependency(&actual, &dep1), IDCU_SUCCESS);
    
    idcu_ModuleDependency dep2 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(2, 0, 0),
        .has_max_version = 0
    };
    TEST_ASSERT_EQUAL(idcu_version_check_dependency(&actual, &dep2), IDCU_ERROR_VERSION_TOO_OLD);
    
    idcu_ModuleDependency dep3 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(1, 0, 0),
        .max_version = idcu_version_create(1, 4, 0),
        .has_max_version = 1
    };
    TEST_ASSERT_EQUAL(idcu_version_check_dependency(&actual, &dep3), IDCU_ERROR_VERSION_TOO_NEW);
    
    idcu_ModuleDependency dep4 = {
        .dependency_name = "test",
        .min_version = idcu_version_create(2, 0, 0),
        .has_max_version = 0
    };
    idcu_ModuleVersion actual_v2 = idcu_version_create(2, 0, 0);
    TEST_ASSERT_EQUAL(idcu_version_check_dependency(&actual_v2, &dep4), IDCU_SUCCESS);
}

int main(void) {
    TEST_INIT();
    
    test_version_create();
    test_version_format();
    test_version_parse();
    test_version_compare();
    test_version_compatibility();
    test_version_check_dependency();
    
    TEST_SUMMARY();
    return TEST_RESULT();
}
