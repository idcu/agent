#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_current_test_passed = 1;

void idcu_test_suite_init(idcu_TestSuite* suite, const char* name)
{
    if (!suite) {
        return;
    }
    memset(suite, 0, sizeof(idcu_TestSuite));
    suite->name = name;
    suite->capacity = 32;
    suite->tests = (idcu_TestCase*)malloc(suite->capacity * sizeof(idcu_TestCase));
    if (!suite->tests) {
        suite->capacity = 0;
    }
}

void idcu_test_suite_destroy(idcu_TestSuite* suite)
{
    if (!suite) {
        return;
    }
    if (suite->tests) {
        free(suite->tests);
        suite->tests = NULL;
    }
    memset(suite, 0, sizeof(idcu_TestSuite));
}

int idcu_test_suite_add_test(idcu_TestSuite* suite, const char* name, idcu_TestFunc func)
{
    if (!suite || !name || !func || !suite->tests) {
        return -1;
    }
    if (suite->count >= suite->capacity) {
        size_t new_cap = suite->capacity * 2;
        idcu_TestCase* new_tests = (idcu_TestCase*)realloc(suite->tests, new_cap * sizeof(idcu_TestCase));
        if (!new_tests) {
            return -1;
        }
        suite->tests = new_tests;
        suite->capacity = new_cap;
    }
    idcu_TestCase* tc = &suite->tests[suite->count];
    tc->name = name;
    tc->func = func;
    tc->passed = 0;
    tc->failed = 0;
    suite->count++;
    return 0;
}

void idcu_test_suite_run(idcu_TestSuite* suite)
{
    if (!suite || !suite->tests) {
        return;
    }
    printf("\n=== Test Suite: %s ===\n", suite->name);
    for (size_t i = 0; i < suite->count; i++) {
        idcu_TestCase* tc = &suite->tests[i];
        printf("  Running: %s... ", tc->name);
        fflush(stdout);
        g_current_test_passed = 1;
        tc->func();
        if (g_current_test_passed) {
            tc->passed = 1;
            suite->total_passed++;
            printf("PASSED\n");
        } else {
            tc->failed = 1;
            suite->total_failed++;
            printf("FAILED\n");
        }
    }
}

void idcu_test_suite_print_summary(idcu_TestSuite* suite)
{
    if (!suite) {
        return;
    }
    printf("\n=== Summary: %s ===\n", suite->name);
    printf("  Total: %zu\n", suite->count);
    printf("  Passed: %d\n", suite->total_passed);
    printf("  Failed: %d\n", suite->total_failed);
}

int idcu_test_suite_get_failures(idcu_TestSuite* suite)
{
    if (!suite) {
        return 0;
    }
    return suite->total_failed;
}

void idcu_test_pass(void)
{
    g_current_test_passed = 1;
}

void idcu_test_fail(const char* file, int line, const char* msg)
{
    g_current_test_passed = 0;
    if (msg) {
        printf("\n    FAIL: %s (%s:%d)\n", msg, file, line);
    } else {
        printf("\n    FAIL: (%s:%d)\n", file, line);
    }
}

void idcu_test_assert(int condition, const char* msg)
{
    if (!condition) {
        idcu_test_fail(__FILE__, __LINE__, msg);
    }
}
