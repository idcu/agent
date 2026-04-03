#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_current_test_passed = 1;

void test_suite_init(TestSuite* suite, const char* name)
{
    if (!suite) {
        return;
    }
    memset(suite, 0, sizeof(TestSuite));
    suite->name = name;
    suite->capacity = 32;
    suite->tests = (TestCase*)malloc(suite->capacity * sizeof(TestCase));
    if (!suite->tests) {
        suite->capacity = 0;
    }
}

void test_suite_destroy(TestSuite* suite)
{
    if (!suite) {
        return;
    }
    if (suite->tests) {
        free(suite->tests);
        suite->tests = NULL;
    }
    memset(suite, 0, sizeof(TestSuite));
}

int test_suite_add_test(TestSuite* suite, const char* name, TestFunc func)
{
    if (!suite || !name || !func || !suite->tests) {
        return -1;
    }
    if (suite->count >= suite->capacity) {
        size_t new_cap = suite->capacity * 2;
        TestCase* new_tests = (TestCase*)realloc(suite->tests, new_cap * sizeof(TestCase));
        if (!new_tests) {
            return -1;
        }
        suite->tests = new_tests;
        suite->capacity = new_cap;
    }
    TestCase* tc = &suite->tests[suite->count];
    tc->name = name;
    tc->func = func;
    tc->passed = 0;
    tc->failed = 0;
    suite->count++;
    return 0;
}

void test_suite_run(TestSuite* suite)
{
    if (!suite || !suite->tests) {
        return;
    }
    printf("\n=== Test Suite: %s ===\n", suite->name);
    for (size_t i = 0; i < suite->count; i++) {
        TestCase* tc = &suite->tests[i];
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

void test_suite_print_summary(TestSuite* suite)
{
    if (!suite) {
        return;
    }
    printf("\n=== Summary: %s ===\n", suite->name);
    printf("  Total: %zu\n", suite->count);
    printf("  Passed: %d\n", suite->total_passed);
    printf("  Failed: %d\n", suite->total_failed);
}

int test_suite_get_failures(TestSuite* suite)
{
    if (!suite) {
        return 0;
    }
    return suite->total_failed;
}

void test_pass(void)
{
    g_current_test_passed = 1;
}

void test_fail(const char* file, int line, const char* msg)
{
    g_current_test_passed = 0;
    if (msg) {
        printf("\n    FAIL: %s (%s:%d)\n", msg, file, line);
    } else {
        printf("\n    FAIL: (%s:%d)\n", file, line);
    }
}

void test_assert(int condition, const char* msg)
{
    if (!condition) {
        test_fail(__FILE__, __LINE__, msg);
    }
}
