#include "idcu/log/log.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    printf("Testing log level operations...\n");

    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    assert(ret == 0);

    idcu_LogLevel current_level = idcu_log_get_level();
    assert(current_level == IDCU_LOG_INFO);
    printf("✓ Initial log level is INFO\n");

    idcu_log_set_level(IDCU_LOG_DEBUG);
    current_level = idcu_log_get_level();
    assert(current_level == IDCU_LOG_DEBUG);
    printf("✓ Log level changed to DEBUG\n");

    idcu_log_set_level(IDCU_LOG_WARN);
    current_level = idcu_log_get_level();
    assert(current_level == IDCU_LOG_WARN);
    printf("✓ Log level changed to WARN\n");

    idcu_log_set_level(IDCU_LOG_ERROR);
    current_level = idcu_log_get_level();
    assert(current_level == IDCU_LOG_ERROR);
    printf("✓ Log level changed to ERROR\n");

    idcu_log_set_level(IDCU_LOG_FATAL);
    current_level = idcu_log_get_level();
    assert(current_level == IDCU_LOG_FATAL);
    printf("✓ Log level changed to FATAL\n");

    idcu_log_shutdown();
    printf("All tests passed!\n");
    return 0;
}
