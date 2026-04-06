#ifndef IDCU_LOG_LOG_H
#define IDCU_LOG_LOG_H

#include "idcu/common/config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

// 日志级别
typedef enum {
    IDCU_LOG_DEBUG   = 0,
    IDCU_LOG_INFO    = 1,
    IDCU_LOG_WARN    = 2,
    IDCU_LOG_ERROR   = 3,
    IDCU_LOG_FATAL   = 4
} idcu_LogLevel;

// 日志输出目标
typedef enum {
    IDCU_LOG_OUTPUT_CONSOLE = 1 << 0,
    IDCU_LOG_OUTPUT_FILE    = 1 << 1,
    IDCU_LOG_OUTPUT_REMOTE  = 1 << 2
} idcu_LogOutput;

// 日志轮转策略
typedef enum {
    IDCU_LOG_ROTATE_NONE   = 0,
    IDCU_LOG_ROTATE_SIZE   = 1,
    IDCU_LOG_ROTATE_TIME   = 2,
    IDCU_LOG_ROTATE_BOTH   = 3
} idcu_LogRotatePolicy;

// 日志配置
typedef struct {
    char filename[256];
    idcu_LogLevel level;
    idcu_LogOutput output;
    idcu_LogRotatePolicy rotate_policy;
    uint64_t max_file_size;    // 最大文件大小（字节）
    uint32_t rotate_interval;   // 轮转间隔（秒）
    uint32_t max_backup_files;  // 最大备份文件数
    char remote_url[256];       // 远程日志服务地址
} idcu_LogConfig;

// 日志初始化
int idcu_log_init(const char* filename, idcu_LogLevel level);

// 使用配置初始化日志
int idcu_log_init_with_config(const idcu_LogConfig* config);

// 获取默认日志配置
void idcu_log_get_default_config(idcu_LogConfig* config);

// 日志关闭
void idcu_log_shutdown(void);

// 设置日志级别
void idcu_log_set_level(idcu_LogLevel level);

// 获取当前日志级别
idcu_LogLevel idcu_log_get_level(void);

// 设置日志文件
int idcu_log_set_file(const char* filename);

// 设置日志输出目标
void idcu_log_set_output(idcu_LogOutput output);

// 设置日志轮转策略
int idcu_log_set_rotate_policy(idcu_LogRotatePolicy policy, uint64_t max_size, uint32_t interval, uint32_t max_backups);

// 手动触发日志轮转
int idcu_log_rotate(void);

// 日志输出接口
void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...);

// 日志宏封装
#define IDCU_LOG_DEBUG(fmt, ...) \
    do { idcu_log_printf(IDCU_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_INFO(fmt, ...) \
    do { idcu_log_printf(IDCU_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_WARN(fmt, ...) \
    do { idcu_log_printf(IDCU_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_WARNING(fmt, ...) IDCU_LOG_WARN(fmt, ##__VA_ARGS__)

#define IDCU_LOG_ERROR(fmt, ...) \
    do { idcu_log_printf(IDCU_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define IDCU_LOG_FATAL(fmt, ...) \
    do { idcu_log_printf(IDCU_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#endif // IDCU_LOG_LOG_H
