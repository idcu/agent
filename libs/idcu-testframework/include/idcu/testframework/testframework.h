#ifndef IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H
#define IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} idcu_TestStats;

typedef void (*idcu_TestFunc)(void);

void idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func);

#define IDCU_TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %s\n", __FILE__, __LINE__, #cond); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_MSG(cond, msg, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %s != %s (expected %d, got %d)\n", \
                    __FILE__, __LINE__, #expected, #actual, (int)(expected), (int)(actual)); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - \"%s\" != \"%s\"\n", \
                    __FILE__, __LINE__, (expected), (actual)); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_NOT_EQUAL(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %s == %s (expected not equal)\n", \
                    __FILE__, __LINE__, #expected, #actual); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_ASSERT_DOUBLE_EQUAL(expected, actual, epsilon) \
    do { \
        double _expected = (expected); \
        double _actual = (actual); \
        double _epsilon = (epsilon); \
        if ((_actual < _expected - _epsilon) || (_actual > _expected + _epsilon)) { \
            fprintf(stderr, "ASSERT FAILED: %s:%d - %g != %g (epsilon %g)\n", \
                    __FILE__, __LINE__, _expected, _actual, _epsilon); \
            idcu_test_fail_current(); \
            return; \
        } \
    } while (0)

#define IDCU_TEST_PASS() \
    do { \
    } while (0)

#define IDCU_TEST_CASE(suite, name) \
    static void _test_##suite##_##name(void); \
    static void _register_test_##suite##_##name(void) __attribute__((constructor)); \
    static void _register_test_##suite##_##name(void) { \
        idcu_test_register(#suite, #name, _test_##suite##_##name); \
    } \
    static void _test_##suite##_##name(void)

void idcu_test_fail_current(void);
int idcu_test_run_all(void);
idcu_TestStats idcu_test_get_stats(void);

#endif
