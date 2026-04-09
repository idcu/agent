#include "idcu/testframework/testframework.h"
#include <string.h>

static int add(int a, int b) {
    return a + b;
}

static const char* get_greeting(void) {
    return "Hello, World!";
}

IDCU_TEST_CASE(Math, AddPositiveNumbers) {
    IDCU_TEST_ASSERT_EQUAL(5, add(2, 3));
    IDCU_TEST_ASSERT_EQUAL(10, add(5, 5));
}

IDCU_TEST_CASE(Math, AddNegativeNumbers) {
    IDCU_TEST_ASSERT_EQUAL(-1, add(2, -3));
    IDCU_TEST_ASSERT_EQUAL(-5, add(-2, -3));
}

IDCU_TEST_CASE(String, Greeting) {
    IDCU_TEST_ASSERT_STRING_EQUAL("Hello, World!", get_greeting());
}

int main(void) {
    return idcu_test_run_all();
}
