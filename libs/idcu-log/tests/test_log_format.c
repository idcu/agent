#include "idcu/log/log.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("Testing log format operations...\n");

    int ret = idcu_log_init(NULL, IDCU_LOG_DEBUG);
    assert(ret == 0);

    printf("✓ Testing different log formats:\n");

    IDCU_LOG_DEBUG("Simple debug message");
    IDCU_LOG_INFO("Integer: %d", 42);
    IDCU_LOG_WARN("Float: %.2f", 3.14);
    IDCU_LOG_ERROR("String: %s", "test");
    IDCU_LOG_DEBUG("Multiple values: %d, %s, %.1f", 100, "hello", 2.5);

    printf("✓ All format specifiers tested\n");

    idcu_log_shutdown();
    printf("All tests passed!\n");
    return 0;
}
