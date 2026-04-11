#include <idcu/testframework/testframework.h>
#include <idcu/log/log.h>
#include <idcu/log/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

IDCU_TEST_CASE(log, init_shutdown_simple) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, init_with_config_null) {
    int ret = idcu_log_init_with_config(NULL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_INVALID_ARG, ret);
}

IDCU_TEST_CASE(log, init_with_config) {
    idcu_LogConfig config;
    memset(&config, 0, sizeof(config));
    config.level = IDCU_LOG_INFO;
    config.output = IDCU_LOG_OUTPUT_CONSOLE;
    
    int ret = idcu_log_init_with_config(&config);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, init_with_file) {
    const char* test_file = "test_log_output.log";
    
    int ret = idcu_log_init(test_file, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_log_shutdown();
    
    FILE* f = fopen(test_file, "r");
    IDCU_TEST_ASSERT(f != NULL);
    if (f) {
        fclose(f);
    }
    
    remove(test_file);
}

IDCU_TEST_CASE(log, set_level) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    idcu_log_set_level(IDCU_LOG_DEBUG);
    idcu_log_set_level(IDCU_LOG_WARN);
    idcu_log_set_level(IDCU_LOG_ERROR);
    idcu_log_set_level(IDCU_LOG_FATAL);
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, basic_logging) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_LOG_INFO("Test info message: %d", 42);
    IDCU_LOG_WARN("Test warning message: %s", "warning");
    IDCU_LOG_ERROR("Test error message: %f", 3.14);
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, debug_filtered) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_LOG_DEBUG("This debug message should be filtered");
    IDCU_LOG_INFO("This info message should be logged");
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, multiple_init) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_LOG_INFO("First init message");
    
    ret = idcu_log_init(NULL, IDCU_LOG_DEBUG);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_LOG_DEBUG("Second init message (should work)");
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, shutdown_without_init) {
    idcu_log_shutdown();
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(log, set_level_without_init) {
    idcu_log_set_level(IDCU_LOG_DEBUG);
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(log, log_without_init) {
    IDCU_LOG_INFO("This should not crash");
    IDCU_TEST_PASS();
}

IDCU_TEST_CASE(log, long_message) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    char long_str[2048];
    memset(long_str, 'A', sizeof(long_str) - 1);
    long_str[sizeof(long_str) - 1] = '\0';
    
    IDCU_LOG_INFO("Long message: %s", long_str);
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, format_specifiers) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_LOG_INFO("Integer: %d, Hex: 0x%x, Float: %.2f", 123, 255, 3.14159);
    IDCU_LOG_INFO("String: %s, Char: %c", "test", 'X');
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, fatal_level) {
    int ret = idcu_log_init(NULL, IDCU_LOG_FATAL);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_LOG_FATAL("Fatal error occurred");
    
    idcu_log_shutdown();
}

IDCU_TEST_CASE(log, all_levels) {
    int ret = idcu_log_init(NULL, IDCU_LOG_DEBUG);
    IDCU_TEST_ASSERT_EQUAL(IDCU_ERR_OK, ret);
    
    IDCU_LOG_DEBUG("Debug message");
    IDCU_LOG_INFO("Info message");
    IDCU_LOG_WARN("Warning message");
    IDCU_LOG_ERROR("Error message");
    IDCU_LOG_FATAL("Fatal message");
    
    idcu_log_shutdown();
}

int main(void) {
    return idcu_test_run_all();
}
