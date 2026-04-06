#include "idcu/log/log.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("Testing log initialization...\n");

    int ret = idcu_log_init(NULL, IDCU_LOG_INFO);
    assert(ret == 0);
    printf("✓ Log initialized with default file and INFO level\n");

    idcu_log_shutdown();
    printf("✓ Log shutdown completed\n");

    printf("All tests passed!\n");
    return 0;
}
