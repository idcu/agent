/*
 * 文件日志示例
 */

#include "idcu/log/log.h"

int main() {
    // 初始化日志，同时输出到控制台和文件
    const char* log_file = "example_file_log.log";
    idcu_log_init(log_file, IDCU_LOG_DEBUG);

    IDCU_LOG_INFO("=== 文件日志示例开始 ===");

    // 输出各种级别的日志
    IDCU_LOG_DEBUG("调试信息 - 这条会输出到控制台和文件");
    IDCU_LOG_INFO("应用程序已启动");
    IDCU_LOG_WARN("内存使用率警告: %d MB", 768);
    IDCU_LOG_ERROR("无法打开配置文件");

    // 更改日志文件
    const char* new_log_file = "example_file_log_new.log";
    IDCU_LOG_INFO("切换到新的日志文件: %s", new_log_file);
    idcu_log_set_file(new_log_file);

    IDCU_LOG_INFO("这条日志会写入新的日志文件");

    // 关闭文件输出，仅输出到控制台
    IDCU_LOG_INFO("关闭文件输出");
    idcu_log_set_file(NULL);

    IDCU_LOG_INFO("这条日志仅输出到控制台");

    // 关闭日志
    IDCU_LOG_INFO("=== 文件日志示例结束 ===");
    idcu_log_shutdown();

    return 0;
}
