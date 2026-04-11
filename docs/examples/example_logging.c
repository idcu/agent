#include <idcu/log/log.h>
#include <stdio.h>

int main(void) {
    printf("=== IDCU Logging Example ===\n\n");
    
    // Example 1: Basic console logging
    printf("Example 1: Basic console logging\n");
    int ret = idcu_log_init(NULL, IDCU_LOG_DEBUG);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init log: %s\n", idcu_error_message(ret));
        return 1;
    }
    
    IDCU_LOG_DEBUG("This is a debug message");
    IDCU_LOG_INFO("This is an info message");
    IDCU_LOG_WARN("This is a warning message");
    IDCU_LOG_ERROR("This is an error message");
    
    idcu_log_shutdown();
    printf("\n");
    
    // Example 2: Log to file
    printf("Example 2: Log to file\n");
    ret = idcu_log_init("example.log", IDCU_LOG_INFO);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init log with file: %s\n", idcu_error_message(ret));
        return 1;
    }
    
    IDCU_LOG_INFO("Application started");
    IDCU_LOG_WARN("Low memory warning");
    IDCU_LOG_ERROR("Failed to connect to database");
    
    idcu_log_shutdown();
    printf("Logs written to example.log\n\n");
    
    // Example 3: Dynamic log level
    printf("Example 3: Dynamic log level\n");
    ret = idcu_log_init(NULL, IDCU_LOG_WARN);
    if (ret != IDCU_ERR_OK) {
        printf("Failed to init log: %s\n", idcu_error_message(ret));
        return 1;
    }
    
    IDCU_LOG_INFO("This info message won't be shown (level is WARN)");
    IDCU_LOG_WARN("This warning will be shown");
    
    printf("Changing log level to INFO...\n");
    idcu_log_set_level(IDCU_LOG_INFO);
    
    IDCU_LOG_INFO("Now this info message is shown");
    
    idcu_log_shutdown();
    
    printf("\n=== Logging Example Complete ===\n");
    return 0;
}
