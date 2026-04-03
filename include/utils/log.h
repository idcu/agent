#ifndef IDCU_UTILS_LOG_H
#define IDCU_UTILS_LOG_H

#include "common/config.h"
#include <stdio.h>
#include <stdarg.h>

// 日志级别
typedef enum {
    IDCU_LOG_DEBUG   = 0,
    IDCU_LOG_INFO    = 1,
    IDCU_LOG_WARN    = 2,
    IDCU_LOG_ERROR   = 3,
    IDCU_LOG_FATAL   = 4
} idcu_LogLevel;

// 日志初始化
int idcu_log_init(const char* filename, idcu_LogLevel level);

// 日志输出接口
void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...);

// 日志宏封装
#define IDCU_LOG_DEBUG(fmt, ...) \
    do { if (IDCU_CONFIG_LOG_LEVEL <= IDCU_LOG_DEBUG) \
        idcu_log_printf(IDCU_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_INFO(fmt, ...) \
    do { if (IDCU_CONFIG_LOG_LEVEL <= IDCU_LOG_INFO) \
        idcu_log_printf(IDCU_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_WARN(fmt, ...) \
    do { if (IDCU_CONFIG_LOG_LEVEL <= IDCU_LOG_WARN) \
        idcu_log_printf(IDCU_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_ERROR(fmt, ...) \
    do { if (IDCU_CONFIG_LOG_LEVEL <= IDCU_LOG_ERROR) \
        idcu_log_printf(IDCU_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_FATAL(fmt, ...) \
    do { if (IDCU_CONFIG_LOG_LEVEL <= IDCU_LOG_FATAL) \
        idcu_log_printf(IDCU_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#endif // IDCU_UTILS_LOG_H
