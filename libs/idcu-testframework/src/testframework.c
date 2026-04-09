#include "idcu/testframework/testframework.h"
#include <stdlib.h>
#include <string.h>

#define MAX_TESTS 1024

typedef struct {
    const char* suite_name;
    const char* test_name;
    idcu_TestFunc func;
    bool failed;
} idcu_TestCase;

static idcu_TestCase g_tests[MAX_TESTS];
static int g_test_count = 0;
static int g_current_test = -1;
static idcu_TestStats g_stats = {0};

void idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func) {
    if (g_test_count >= MAX_TESTS) {
        fprintf(stderr, "Too many tests! Maximum is %d\n", MAX_TESTS);
        return;
    }
    
    g_tests[g_test_count].suite_name = suite_name;
    g_tests[g_test_count].test_name = test_name;
    g_tests[g_test_count].func = func;
    g_tests[g_test_count].failed = false;
    g_test_count++;
}

void idcu_test_fail_current(void) {
    if (g_current_test >= 0 && g_current_test < g_test_count) {
        g_tests[g_current_test].failed = true;
    }
}

int idcu_test_run_all(void) {
    printf("========================================\n");
    printf("Running %d tests...\n", g_test_count);
    printf("========================================\n\n");
    
    g_stats.total_tests = g_test_count;
    g_stats.passed_tests = 0;
    g_stats.failed_tests = 0;
    
    const char* current_suite = NULL;
    
    for (int i = 0; i < g_test_count; i++) {
        if (current_suite == NULL || strcmp(g_tests[i].suite_name, current_suite) != 0) {
            current_suite = g_tests[i].suite_name;
            printf("[Suite: %s]\n", current_suite);
        }
        
        printf("  Running: %s... ", g_tests[i].test_name);
        fflush(stdout);
        
        g_current_test = i;
        g_tests[i].failed = false;
        g_tests[i].func();
        
        if (g_tests[i].failed) {
            printf("FAILED\n");
            g_stats.failed_tests++;
        } else {
            printf("PASSED\n");
            g_stats.passed_tests++;
        }
    }
    
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total:  %d\n", g_stats.total_tests);
    printf("  Passed: %d\n", g_stats.passed_tests);
    printf("  Failed: %d\n", g_stats.failed_tests);
    printf("========================================\n");
    
    return g_stats.failed_tests;
}

idcu_TestStats idcu_test_get_stats(void) {
    return g_stats;
}
