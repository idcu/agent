#ifndef LOG_H
#define LOG_H

#include "config.h"
#include <stdio.h>
#include <stdarg.h>

// 日志级别
typedef enum {
    LOG_DEBUG   = 0,
    LOG_INFO    = 1,
    LOG_WARN    = 2,
    LOG_ERROR   = 3,
    LOG_FATAL   = 4
} LogLevel;

// 日志初始化
int log_init(const char* filename, LogLevel level);

// 日志输出接口
void log_printf(LogLevel level, const char* file, int line, const char* fmt, ...);

// 日志宏封装
#define LOG_DEBUG(fmt, ...) \
    do { if (CONFIG_LOG_LEVEL <= LOG_DEBUG) \
        log_printf(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_INFO(fmt, ...) \
    do { if (CONFIG_LOG_LEVEL <= LOG_INFO) \
        log_printf(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_WARN(fmt, ...) \
    do { if (CONFIG_LOG_LEVEL <= LOG_WARN) \
        log_printf(LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_ERROR(fmt, ...) \
    do { if (CONFIG_LOG_LEVEL <= LOG_ERROR) \
        log_printf(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_FATAL(fmt, ...) \
    do { if (CONFIG_LOG_LEVEL <= LOG_FATAL) \
        log_printf(LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#endif // LOG_H