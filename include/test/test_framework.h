#ifndef IDCU_TEST_TEST_FRAMEWORK_H
#define IDCU_TEST_TEST_FRAMEWORK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // IDCU_TEST_TEST_FRAMEWORK_H

typedef void (*idcu_TestFunc)(void);

typedef struct {
    const char* name;
    idcu_TestFunc func;
    int passed;
    int failed;
} idcu_TestCase;

typedef struct {
    const char* name;
    idcu_TestCase* tests;
    size_t count;
    size_t capacity;
    int total_passed;
    int total_failed;
} idcu_TestSuite;

void idcu_test_suite_init(idcu_TestSuite* suite, const char* name);
void idcu_test_suite_destroy(idcu_TestSuite* suite);
int idcu_test_suite_add_test(idcu_TestSuite* suite, const char* name, idcu_TestFunc func);
void idcu_test_suite_run(idcu_TestSuite* suite);
void idcu_test_suite_print_summary(idcu_TestSuite* suite);
int idcu_test_suite_get_failures(idcu_TestSuite* suite);

void idcu_test_pass(void);
void idcu_test_fail(const char* file, int line, const char* msg);
void idcu_test_assert(int condition, const char* msg);

#define IDCU_TEST_PASS() idcu_test_pass()
#define IDCU_TEST_FAIL(msg) idcu_test_fail(__FILE__, __LINE__, msg)
#define IDCU_TEST_ASSERT(cond, msg) idcu_test_assert(cond, msg)

#ifdef __cplusplus
}
#endif // IDCU_TEST_TEST_FRAMEWORK_H

#endif // IDCU_TEST_TEST_FRAMEWORK_H
