#include <idcu/testframework/test.h>
#include <idcu/log/log.h>
#include <stdio.h>
#include <string.h>

TEST_CASE(log_init_basic) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    idcu_log_shutdown();
}

TEST_CASE(log_init_with_config) {
    idcu_LogConfig config = {0};
    config.level = IDCU_LOG_DEBUG;
    config.output = IDCU_LOG_OUTPUT_CONSOLE;
    
    int ret = idcu_log_init_with_config(&config);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    idcu_log_shutdown();
}

TEST_CASE(log_init_null_config) {
    int ret = idcu_log_init_with_config(NULL);
    TEST_ASSERT(ret != IDCU_ERR_OK);
}

TEST_CASE(log_set_level) {
    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    
    idcu_log_set_level(IDCU_LOG_DEBUG);
    idcu_log_shutdown();
}

TEST_CASE(log_printf_basic) {
    int ret = idcu_log_init(NULL, IDCU_LOG_DEBUG);
    TEST_ASSERT(ret == IDCU_ERR_OK);
    
    IDCU_LOG_DEBUG("Debug message: %d", 123);
    IDCU_LOG_INFO("Info message: %s", "test");
    IDCU_LOG_WARN("Warning message");
    IDCU_LOG_ERROR("Error: %d", 404);
    IDCU_LOG_FATAL("Fatal: %s", "crash");
    
    idcu_log_shutdown();
}

TEST_SUITE_BEGIN(log_suite)
    TEST_SUITE_ADD(log_init_basic)
    TEST_SUITE_ADD(log_init_with_config)
    TEST_SUITE_ADD(log_init_null_config)
    TEST_SUITE_ADD(log_set_level)
    TEST_SUITE_ADD(log_printf_basic)
TEST_SUITE_END()

TEST_MAIN(log_suite)
