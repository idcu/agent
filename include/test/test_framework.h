#ifndef IDCU_TEST_TEST_FRAMEWORK_H
#define IDCU_TEST_TEST_FRAMEWORK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_TEST_TEST_FRAMEWORK_H

typedef void (*TestFunc)(void);

typedef struct {
    const char* name;
    TestFunc func;
    int passed;
    int failed;
} TestCase;

typedef struct {
    const char* name;
    TestCase* tests;
    size_t count;
    size_t capacity;
    int total_passed;
    int total_failed;
} TestSuite;

void test_suite_init(TestSuite* suite, const char* name);
void test_suite_destroy(TestSuite* suite);
int test_suite_add_test(TestSuite* suite, const char* name, TestFunc func);
void test_suite_run(TestSuite* suite);
void test_suite_print_summary(TestSuite* suite);
int test_suite_get_failures(TestSuite* suite);

void test_pass(void);
void test_fail(const char* file, int line, const char* msg);
void test_assert(int condition, const char* msg);

#define TEST_PASS() test_pass()
#define TEST_FAIL(msg) test_fail(__FILE__, __LINE__, msg)
#define TEST_ASSERT(cond, msg) test_assert(cond, msg)

#ifdef __cplusplus
}
#endif // IDCU_TEST_TEST_FRAMEWORK_H

#endif // IDCU_TEST_TEST_FRAMEWORK_H
