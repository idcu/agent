#include "idcu/testframework/testframework.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <inttypes.h>
#include <windows.h>

#define IDCU_MAX_TEST_SUITES 64
#define IDCU_MAX_TESTS_PER_SUITE 256

typedef struct idcu_Test {
    char name[IDCU_TEST_NAME_MAX];
    idcu_TestFunc func;
    idcu_TestResult result;
} idcu_Test;

typedef struct idcu_TestSuite {
    char name[IDCU_TEST_SUITE_NAME_MAX];
    idcu_TestSuiteSetupFunc setup;
    idcu_TestSuiteTeardownFunc teardown;
    idcu_Test tests[IDCU_MAX_TESTS_PER_SUITE];
    size_t test_count;
    idcu_TestSuiteResult result;
} idcu_TestSuite;

static idcu_TestSuite g_suites[IDCU_MAX_TEST_SUITES];
static size_t g_suite_count = 0;
static int g_initialized = 0;
static idcu_TestSuite* g_current_suite = NULL;
static idcu_Test* g_current_test = NULL;

static uint64_t get_time_ms(void) {
#ifdef _WIN32
    return (uint64_t)GetTickCount();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
#endif
}

void idcu_test_framework_init(void) {
    if (g_initialized) {
        IDCU_LOG_WARNING("[testframework] Already initialized");
        return;
    }

    memset(g_suites, 0, sizeof(g_suites));
    g_suite_count = 0;
    g_initialized = 1;
    g_current_suite = NULL;
    g_current_test = NULL;

    IDCU_LOG_INFO("[testframework] Initialized");
}

void idcu_test_framework_shutdown(void) {
    if (!g_initialized) {
        return;
    }

    for (size_t i = 0; i < g_suite_count; i++) {
        idcu_TestSuite* suite = &g_suites[i];
        if (suite->result.tests) {
            free(suite->result.tests);
            suite->result.tests = NULL;
        }
    }

    memset(g_suites, 0, sizeof(g_suites));
    g_suite_count = 0;
    g_initialized = 0;

    IDCU_LOG_INFO("[testframework] Shutdown");
}

int idcu_test_suite_register(const char* name, idcu_TestSuiteSetupFunc setup, idcu_TestSuiteTeardownFunc teardown) {
    if (!g_initialized || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }
    if (g_suite_count >= IDCU_MAX_TEST_SUITES) {
        return IDCU_ERR_LIMIT_EXCEEDED;
    }

    for (size_t i = 0; i < g_suite_count; i++) {
        if (strcmp(g_suites[i].name, name) == 0) {
            return IDCU_ERR_ALREADY_EXISTS;
        }
    }

    idcu_TestSuite* suite = &g_suites[g_suite_count];
    strncpy(suite->name, name, IDCU_TEST_SUITE_NAME_MAX - 1);
    suite->name[IDCU_TEST_SUITE_NAME_MAX - 1] = '\0';
    suite->setup = setup;
    suite->teardown = teardown;
    suite->test_count = 0;
    suite->result.tests = NULL;

    g_suite_count++;
    IDCU_LOG_INFO("[testframework] Registered test suite: %s", name);
    return IDCU_ERR_OK;
}

int idcu_test_suite_unregister(const char* name) {
    if (!g_initialized || !name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (size_t i = 0; i < g_suite_count; i++) {
        idcu_TestSuite* suite = &g_suites[i];
        if (strcmp(suite->name, name) == 0) {
            if (suite->result.tests) {
                free(suite->result.tests);
            }
            if (i < g_suite_count - 1) {
                memmove(&g_suites[i], &g_suites[i + 1],
                        (g_suite_count - i - 1) * sizeof(idcu_TestSuite));
            }
            memset(&g_suites[g_suite_count - 1], 0, sizeof(idcu_TestSuite));
            g_suite_count--;

            IDCU_LOG_INFO("[testframework] Unregistered test suite: %s", name);
            return IDCU_ERR_OK;
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

int idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func) {
    if (!g_initialized || !suite_name || !test_name || !func) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_TestSuite* suite = NULL;
    for (size_t i = 0; i < g_suite_count; i++) {
        if (strcmp(g_suites[i].name, suite_name) == 0) {
            suite = &g_suites[i];
            break;
        }
    }

    if (!suite) {
        return IDCU_ERR_NOT_FOUND;
    }
    if (suite->test_count >= IDCU_MAX_TESTS_PER_SUITE) {
        return IDCU_ERR_LIMIT_EXCEEDED;
    }

    idcu_Test* test = &suite->tests[suite->test_count];
    strncpy(test->name, test_name, IDCU_TEST_NAME_MAX - 1);
    test->name[IDCU_TEST_NAME_MAX - 1] = '\0';
    test->func = func;
    memset(&test->result, 0, sizeof(test->result));

    suite->test_count++;
    IDCU_LOG_DEBUG("[testframework] Registered test: %s.%s", suite_name, test_name);
    return IDCU_ERR_OK;
}

static int run_test(idcu_TestSuite* suite, idcu_Test* test) {
    g_current_suite = suite;
    g_current_test = test;

    memset(&test->result, 0, sizeof(test->result));
    strncpy(test->result.name, test->name, IDCU_TEST_NAME_MAX - 1);
    test->result.name[IDCU_TEST_NAME_MAX - 1] = '\0';
    test->result.status = IDCU_TEST_STATUS_PASSED;

    uint64_t start = get_time_ms();

    if (suite->setup) {
        suite->setup();
    }

    if (test->func) {
        test->func();
    }

    if (suite->teardown) {
        suite->teardown();
    }

    test->result.duration_ms = get_time_ms() - start;

    const char* status_str = test->result.status == IDCU_TEST_STATUS_PASSED ? "PASSED" :
                             test->result.status == IDCU_TEST_STATUS_FAILED ? "FAILED" :
                             test->result.status == IDCU_TEST_STATUS_SKIPPED ? "SKIPPED" : "ERROR";

    IDCU_LOG_INFO("[testframework] %s.%s: %s (%" PRIu64 " ms)",
                 suite->name, test->name, status_str, test->result.duration_ms);

    g_current_test = NULL;
    g_current_suite = NULL;
    return IDCU_ERR_OK;
}

static int run_test_suite(idcu_TestSuite* suite) {
    if (suite->result.tests) {
        free(suite->result.tests);
    }

    suite->result.tests = (idcu_TestResult*)calloc(suite->test_count, sizeof(idcu_TestResult));
    if (!suite->result.tests) {
        return IDCU_ERR_NO_MEMORY;
    }

    strncpy(suite->result.name, suite->name, IDCU_TEST_SUITE_NAME_MAX - 1);
    suite->result.name[IDCU_TEST_SUITE_NAME_MAX - 1] = '\0';
    suite->result.test_count = suite->test_count;
    suite->result.passed_count = 0;
    suite->result.failed_count = 0;
    suite->result.skipped_count = 0;
    suite->result.total_duration_ms = 0;

    for (size_t i = 0; i < suite->test_count; i++) {
        run_test(suite, &suite->tests[i]);
        memcpy(&suite->result.tests[i], &suite->tests[i].result, sizeof(idcu_TestResult));

        switch (suite->tests[i].result.status) {
            case IDCU_TEST_STATUS_PASSED:
                suite->result.passed_count++;
                break;
            case IDCU_TEST_STATUS_FAILED:
                suite->result.failed_count++;
                break;
            case IDCU_TEST_STATUS_SKIPPED:
                suite->result.skipped_count++;
                break;
            default:
                suite->result.failed_count++;
                break;
        }
        suite->result.total_duration_ms += suite->tests[i].result.duration_ms;
    }

    IDCU_LOG_INFO("[testframework] Suite %s complete: %zu passed, %zu failed, %zu skipped (%" PRIu64 " ms)",
                 suite->name, suite->result.passed_count, suite->result.failed_count,
                 suite->result.skipped_count, suite->result.total_duration_ms);

    return IDCU_ERR_OK;
}

int idcu_test_run_all(void) {
    if (!g_initialized) {
        return IDCU_ERR_NOT_INITIALIZED;
    }

    IDCU_LOG_INFO("[testframework] Running all tests (%zu suites)", g_suite_count);

    for (size_t i = 0; i < g_suite_count; i++) {
        run_test_suite(&g_suites[i]);
    }

    size_t total = 0, passed = 0, failed = 0, skipped = 0;
    idcu_test_get_total_result(&total, &passed, &failed, &skipped);

    IDCU_LOG_INFO("[testframework] All tests complete: %zu passed, %zu failed, %zu skipped",
                 passed, failed, skipped);

    return IDCU_ERR_OK;
}

int idcu_test_run_suite(const char* suite_name) {
    if (!g_initialized || !suite_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (size_t i = 0; i < g_suite_count; i++) {
        if (strcmp(g_suites[i].name, suite_name) == 0) {
            return run_test_suite(&g_suites[i]);
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

int idcu_test_run_single(const char* suite_name, const char* test_name) {
    if (!g_initialized || !suite_name || !test_name) {
        return IDCU_ERR_INVALID_PARAM;
    }

    idcu_TestSuite* suite = NULL;
    for (size_t i = 0; i < g_suite_count; i++) {
        if (strcmp(g_suites[i].name, suite_name) == 0) {
            suite = &g_suites[i];
            break;
        }
    }

    if (!suite) {
        return IDCU_ERR_NOT_FOUND;
    }

    for (size_t i = 0; i < suite->test_count; i++) {
        if (strcmp(suite->tests[i].name, test_name) == 0) {
            return run_test(suite, &suite->tests[i]);
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

int idcu_test_get_suite_result(const char* suite_name, idcu_TestSuiteResult* result) {
    if (!g_initialized || !suite_name || !result) {
        return IDCU_ERR_INVALID_PARAM;
    }

    for (size_t i = 0; i < g_suite_count; i++) {
        if (strcmp(g_suites[i].name, suite_name) == 0) {
            memcpy(result, &g_suites[i].result, sizeof(idcu_TestSuiteResult));
            return IDCU_ERR_OK;
        }
    }

    return IDCU_ERR_NOT_FOUND;
}

int idcu_test_get_total_result(size_t* total_tests, size_t* total_passed, size_t* total_failed, size_t* total_skipped) {
    if (!g_initialized || !total_tests || !total_passed || !total_failed || !total_skipped) {
        return IDCU_ERR_INVALID_PARAM;
    }

    *total_tests = 0;
    *total_passed = 0;
    *total_failed = 0;
    *total_skipped = 0;

    for (size_t i = 0; i < g_suite_count; i++) {
        *total_tests += g_suites[i].result.test_count;
        *total_passed += g_suites[i].result.passed_count;
        *total_failed += g_suites[i].result.failed_count;
        *total_skipped += g_suites[i].result.skipped_count;
    }

    return IDCU_ERR_OK;
}

static void set_test_status(idcu_TestStatus status, const char* format, va_list args) {
    if (g_current_test) {
        g_current_test->result.status = status;
        vsnprintf(g_current_test->result.error_message, sizeof(g_current_test->result.error_message), format, args);
    }
}

void idcu_test_assert(bool condition, const char* message, ...) {
    if (!condition) {
        va_list args;
        va_start(args, message);
        set_test_status(IDCU_TEST_STATUS_FAILED, message, args);
        va_end(args);
    }
}

void idcu_test_assert_int_eq(int64_t expected, int64_t actual, const char* message, ...) {
    if (expected != actual) {
        char full_msg[2048];
        va_list args;
        va_start(args, message);
        vsnprintf(full_msg, sizeof(full_msg) - 64, message, args);
        va_end(args);
        snprintf(full_msg + strlen(full_msg), sizeof(full_msg) - strlen(full_msg),
                 " (expected: %" PRId64 ", actual: %" PRId64 ")", expected, actual);
        if (g_current_test) {
            g_current_test->result.status = IDCU_TEST_STATUS_FAILED;
            strncpy(g_current_test->result.error_message, full_msg, sizeof(g_current_test->result.error_message) - 1);
        }
    }
}

void idcu_test_assert_int_ne(int64_t expected, int64_t actual, const char* message, ...) {
    if (expected == actual) {
        va_list args;
        va_start(args, message);
        set_test_status(IDCU_TEST_STATUS_FAILED, message, args);
        va_end(args);
    }
}

void idcu_test_assert_str_eq(const char* expected, const char* actual, const char* message, ...) {
    if (!expected || !actual || strcmp(expected, actual) != 0) {
        va_list args;
        va_start(args, message);
        set_test_status(IDCU_TEST_STATUS_FAILED, message, args);
        va_end(args);
    }
}

void idcu_test_assert_str_ne(const char* expected, const char* actual, const char* message, ...) {
    if (expected && actual && strcmp(expected, actual) == 0) {
        va_list args;
        va_start(args, message);
        set_test_status(IDCU_TEST_STATUS_FAILED, message, args);
        va_end(args);
    }
}

void idcu_test_assert_ptr_eq(const void* expected, const void* actual, const char* message, ...) {
    if (expected != actual) {
        va_list args;
        va_start(args, message);
        set_test_status(IDCU_TEST_STATUS_FAILED, message, args);
        va_end(args);
    }
}

void idcu_test_assert_ptr_ne(const void* expected, const void* actual, const char* message, ...) {
    if (expected == actual) {
        va_list args;
        va_start(args, message);
        set_test_status(IDCU_TEST_STATUS_FAILED, message, args);
        va_end(args);
    }
}

void idcu_test_fail(const char* message, ...) {
    va_list args;
    va_start(args, message);
    set_test_status(IDCU_TEST_STATUS_FAILED, message, args);
    va_end(args);
}

void idcu_test_skip(const char* message, ...) {
    va_list args;
    va_start(args, message);
    set_test_status(IDCU_TEST_STATUS_SKIPPED, message, args);
    va_end(args);
}
