/*
 * 简单日志示例
 */

#include "idcu/log/log.h"

int main() {
    // 初始化日志，输出到控制台，级别为 DEBUG
    idcu_log_init(NULL, IDCU_LOG_DEBUG);

    // 输出各种级别的日志
    IDCU_LOG_DEBUG("This is a debug message");
    IDCU_LOG_INFO("Application started successfully");
    IDCU_LOG_WARN("Low memory warning: %d MB", 512);
    IDCU_LOG_ERROR("Failed to load configuration file");

    // 更改日志级别
    idcu_log_set_level(IDCU_LOG_WARN);
    IDCU_LOG_DEBUG("This debug message should NOT be shown");
    IDCU_LOG_WARN("This warning should be shown");

    // 关闭日志
    idcu_log_shutdown();

    return 0;
}
