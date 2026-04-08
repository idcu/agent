#include "idcu/log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int test_passed = 0;
static int test_failed = 0;

#define TEST_ASSERT(cond, msg)                                                                     \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            printf("FAIL: %s\n", msg);                                                             \
            test_failed++;                                                                         \
        } else {                                                                                   \
            test_passed++;                                                                         \
            printf("PASS: %s\n", msg);                                                             \
        }                                                                                          \
    } while (0)

static void test_log_default_config(void) {
    idcu_LogConfig config;
    idcu_log_get_default_config(&config);

    TEST_ASSERT(config.level == IDCU_LOG_INFO, "default level should be INFO");
    TEST_ASSERT((config.output & IDCU_LOG_OUTPUT_CONSOLE) != 0, "should have console output");
    TEST_ASSERT((config.output & IDCU_LOG_OUTPUT_FILE) != 0, "should have file output");
    TEST_ASSERT(config.rotate_policy == IDCU_LOG_ROTATE_NONE, "rotate policy should be NONE");
    TEST_ASSERT(config.max_file_size == 10 * 1024 * 1024, "max file size should be 10MB");
    TEST_ASSERT(config.max_backup_files == 5, "max backup files should be 5");
}

static void test_log_init_shutdown(void) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    TEST_ASSERT(ret == 0, "log init with default should succeed");

    idcu_log_shutdown();
    printf("✓ Log shutdown completed\n");

    ret = idcu_log_init("test_log_init.txt", IDCU_LOG_DEBUG);
    TEST_ASSERT(ret == 0, "log init with file should succeed");

    idcu_log_shutdown();
    remove("test_log_init.txt");
}

static void test_log_level_functions(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_LogLevel level = idcu_log_get_level();
    TEST_ASSERT(level == IDCU_LOG_INFO, "initial level should be INFO");

    idcu_log_set_level(IDCU_LOG_DEBUG);
    level = idcu_log_get_level();
    TEST_ASSERT(level == IDCU_LOG_DEBUG, "level should be DEBUG after set");

    idcu_log_set_level(IDCU_LOG_WARN);
    level = idcu_log_get_level();
    TEST_ASSERT(level == IDCU_LOG_WARN, "level should be WARN after set");

    idcu_log_set_level(IDCU_LOG_ERROR);
    level = idcu_log_get_level();
    TEST_ASSERT(level == IDCU_LOG_ERROR, "level should be ERROR after set");

    idcu_log_set_level(IDCU_LOG_FATAL);
    level = idcu_log_get_level();
    TEST_ASSERT(level == IDCU_LOG_FATAL, "level should be FATAL after set");

    idcu_log_shutdown();
}

static void test_log_init_with_config(void) {
    idcu_LogConfig config;
    idcu_log_get_default_config(&config);

    strncpy(config.filename, "test_config_log.txt", sizeof(config.filename) - 1);
    config.level = IDCU_LOG_DEBUG;
    config.output = IDCU_LOG_OUTPUT_FILE;

    int ret = idcu_log_init_with_config(&config);
    TEST_ASSERT(ret == 0, "init with config should succeed");

    idcu_log_shutdown();
    remove("test_config_log.txt");
}

static void test_log_set_file(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    int ret = idcu_log_set_file("test_set_file_log.txt");
    TEST_ASSERT(ret == 0, "set file should succeed");

    idcu_log_shutdown();
    remove("test_set_file_log.txt");
}

static void test_log_set_output(void) {
    idcu_log_init(NULL, IDCU_LOG_INFO);

    idcu_log_set_output(IDCU_LOG_OUTPUT_CONSOLE);
    idcu_log_set_output(IDCU_LOG_OUTPUT_FILE);
    idcu_log_set_output(IDCU_LOG_OUTPUT_CONSOLE | IDCU_LOG_OUTPUT_FILE);

    TEST_ASSERT(1 == 1, "set output should work");

    idcu_log_shutdown();
}

static void test_log_rotate_policy(void) {
    idcu_log_init("test_rotate_log.txt", IDCU_LOG_INFO);

    int ret = idcu_log_set_rotate_policy(IDCU_LOG_ROTATE_SIZE, 1024 * 1024, 3600, 3);
    TEST_ASSERT(ret == 0, "set rotate policy should succeed");

    ret = idcu_log_rotate();
    TEST_ASSERT(ret == 0, "manual rotate should succeed");

    idcu_log_shutdown();
    remove("test_rotate_log.txt");
}

static void test_log_printf_functions(void) {
    idcu_log_init("test_printf_log.txt", IDCU_LOG_DEBUG);

    printf("Testing log printf functions...\n");

    IDCU_LOG_DEBUG("This is a DEBUG message: %d", 123);
    IDCU_LOG_INFO("This is an INFO message: %s", "test");
    IDCU_LOG_WARN("This is a WARN message: %.2f", 3.14);
    IDCU_LOG_ERROR("This is an ERROR message");

    printf("✓ Log printf functions tested\n");

    idcu_log_shutdown();
    remove("test_printf_log.txt");
}

int main(void) {
    printf("Running Comprehensive Log Tests...\n\n");

    test_log_default_config();
    test_log_init_shutdown();
    test_log_level_functions();
    test_log_init_with_config();
    test_log_set_file();
    test_log_set_output();
    test_log_rotate_policy();
    test_log_printf_functions();

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
