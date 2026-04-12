#ifndef IDCU_LOG_LOG_H
#define IDCU_LOG_LOG_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/log/types.h>
#include <stdarg.h>

/**
 * @file log.h
 * @brief IDCU 日志系统
 *
 * 提供结构化日志记录功能，支持多级别日志、
 * 文件和控制台输出、彩色终端显示等特性。
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化日志系统
 * @param filename 日志文件名，NULL 表示仅输出到控制台
 * @param level 初始日志级别
 * @return IDCU_ERR_OK 成功，其他值表示错误
 */
int idcu_log_init(const char* filename, idcu_LogLevel level);

/**
 * @brief 使用配置初始化日志系统
 * @param config 日志配置结构体
 * @return IDCU_ERR_OK 成功，其他值表示错误
 */
int idcu_log_init_with_config(const idcu_LogConfig* config);

/**
 * @brief 关闭日志系统
 *
 * 刷新所有未写入的日志并关闭日志文件。
 */
void idcu_log_shutdown(void);

/**
 * @brief 设置当前日志级别
 * @param level 新的日志级别
 */
void idcu_log_set_level(idcu_LogLevel level);

/**
 * @brief 打印日志消息（内部函数）
 * @param level 日志级别
 * @param file 源文件名
 * @param line 源文件行号
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...);

/**
 * @brief 记录 DEBUG 级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#define IDCU_LOG_DEBUG(fmt, ...) idcu_log_printf(IDCU_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief 记录 INFO 级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#define IDCU_LOG_INFO(fmt, ...)  idcu_log_printf(IDCU_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief 记录 WARN 级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#define IDCU_LOG_WARN(fmt, ...)  idcu_log_printf(IDCU_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief 记录 ERROR 级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#define IDCU_LOG_ERROR(fmt, ...) idcu_log_printf(IDCU_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief 记录 FATAL 级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#define IDCU_LOG_FATAL(fmt, ...) idcu_log_printf(IDCU_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
