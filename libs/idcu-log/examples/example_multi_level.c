/*
 * 多级别日志示例
 */

#include "idcu/log/log.h"

void demo_log_levels(idcu_LogLevel level, const char* desc) {
    IDCU_LOG_INFO("--- 当前日志级别: %s ---", desc);

    IDCU_LOG_DEBUG("DEBUG 级别的消息");
    IDCU_LOG_INFO("INFO 级别的消息");
    IDCU_LOG_WARN("WARN 级别的消息");
    IDCU_LOG_ERROR("ERROR 级别的消息");
}

int main() {
    IDCU_LOG_INFO("=== 多级别日志示例开始 ===");

    // 测试所有级别
    idcu_log_init(NULL, IDCU_LOG_DEBUG);

    demo_log_levels(IDCU_LOG_DEBUG, "DEBUG (显示所有)");

    idcu_log_set_level(IDCU_LOG_INFO);
    demo_log_levels(IDCU_LOG_INFO, "INFO (显示 INFO, WARN, ERROR)");

    idcu_log_set_level(IDCU_LOG_WARN);
    demo_log_levels(IDCU_LOG_WARN, "WARN (显示 WARN, ERROR)");

    idcu_log_set_level(IDCU_LOG_ERROR);
    demo_log_levels(IDCU_LOG_ERROR, "ERROR (仅显示 ERROR)");

    // 展示如何获取当前日志级别
    idcu_LogLevel current_level = idcu_log_get_level();
    IDCU_LOG_INFO("当前日志级别数值: %d", current_level);

    IDCU_LOG_INFO("=== 多级别日志示例结束 ===");
    idcu_log_shutdown();

    return 0;
}
