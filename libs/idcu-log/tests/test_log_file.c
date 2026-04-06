#include "idcu/log/log.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TEST_LOG_FILE "test_log_output.txt"

int main(void) {
    printf("Testing log file operations...\n");

    remove(TEST_LOG_FILE);

    int ret = idcu_log_init(TEST_LOG_FILE, IDCU_LOG_DEBUG);
    assert(ret == 0);
    printf("✓ Log initialized with file '%s'\n", TEST_LOG_FILE);

    IDCU_LOG_INFO("Test info message");
    IDCU_LOG_WARN("Test warning message");
    IDCU_LOG_ERROR("Test error message");

    idcu_log_shutdown();

    FILE* fp = fopen(TEST_LOG_FILE, "r");
    assert(fp != NULL);
    printf("✓ Log file '%s' created successfully\n", TEST_LOG_FILE);

    char buffer[1024];
    int lines = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        lines++;
    }
    fclose(fp);

    assert(lines >= 3);
    printf("✓ Log file contains %d lines (expected >= 3)\n", lines);

    remove(TEST_LOG_FILE);
    printf("✓ Test log file cleaned up\n");

    printf("All tests passed!\n");
    return 0;
}
