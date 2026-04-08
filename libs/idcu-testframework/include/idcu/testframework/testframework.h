#ifndef IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H
#define IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H

#include "idcu/common/error_code.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IDCU_TEST_NAME_MAX       256
#define IDCU_TEST_SUITE_NAME_MAX 256

typedef enum
{
    IDCU_TEST_STATUS_PASSED = 0,
    IDCU_TEST_STATUS_FAILED,
    IDCU_TEST_STATUS_SKIPPED,
    IDCU_TEST_STATUS_ERROR
} idcu_TestStatus;

typedef struct idcu_TestResult
{
    char            name[IDCU_TEST_NAME_MAX];
    idcu_TestStatus status;
    char            error_message[1024];
    uint64_t        duration_ms;
} idcu_TestResult;

typedef struct idcu_TestSuiteResult
{
    char             name[IDCU_TEST_SUITE_NAME_MAX];
    idcu_TestResult* tests;
    size_t           test_count;
    size_t           passed_count;
    size_t           failed_count;
    size_t           skipped_count;
    uint64_t         total_duration_ms;
} idcu_TestSuiteResult;

typedef void (*idcu_TestFunc)(void);
typedef void (*idcu_TestSuiteSetupFunc)(void);
typedef void (*idcu_TestSuiteTeardownFunc)(void);

void idcu_test_framework_init(void);
void idcu_test_framework_shutdown(void);

int idcu_test_suite_register(const char* name, idcu_TestSuiteSetupFunc setup,
                             idcu_TestSuiteTeardownFunc teardown);
int idcu_test_suite_unregister(const char* name);

int idcu_test_register(const char* suite_name, const char* test_name, idcu_TestFunc func);

int idcu_test_run_all(void);
int idcu_test_run_suite(const char* suite_name);
int idcu_test_run_single(const char* suite_name, const char* test_name);

int idcu_test_get_suite_result(const char* suite_name, idcu_TestSuiteResult* result);
int idcu_test_get_total_result(size_t* total_tests, size_t* total_passed, size_t* total_failed,
                               size_t* total_skipped);

void idcu_test_assert(bool condition, const char* message, ...);
void idcu_test_assert_int_eq(int64_t expected, int64_t actual, const char* message, ...);
void idcu_test_assert_int_ne(int64_t expected, int64_t actual, const char* message, ...);
void idcu_test_assert_str_eq(const char* expected, const char* actual, const char* message, ...);
void idcu_test_assert_str_ne(const char* expected, const char* actual, const char* message, ...);
void idcu_test_assert_ptr_eq(const void* expected, const void* actual, const char* message, ...);
void idcu_test_assert_ptr_ne(const void* expected, const void* actual, const char* message, ...);
void idcu_test_fail(const char* message, ...);
void idcu_test_skip(const char* message, ...);

#define IDCU_TEST_ASSERT(cond) idcu_test_assert((cond), #cond " failed at " __FILE__ ":" __LINE__)
#define IDCU_TEST_ASSERT_EQ(expected, actual)     \
    idcu_test_assert_int_eq((expected), (actual), \
                            #expected " != " #actual " at " __FILE__ ":" __LINE__)
#define IDCU_TEST_ASSERT_STR_EQ(expected, actual) \
    idcu_test_assert_str_eq((expected), (actual), "Strings not equal at " __FILE__ ":" __LINE__)
#define IDCU_TEST_FAIL(msg) idcu_test_fail(msg " at " __FILE__ ":" __LINE__)

#endif  // IDCU_TESTFRAMEWORK_TESTFRAMEWORK_H
