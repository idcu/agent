#ifndef IDCU_LOG_LOG_H
#define IDCU_LOG_LOG_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/log/types.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_log_init(const char* filename, idcu_LogLevel level);

int idcu_log_init_with_config(const idcu_LogConfig* config);

void idcu_log_shutdown(void);

void idcu_log_set_level(idcu_LogLevel level);

void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...);

#define IDCU_LOG_DEBUG(fmt, ...) idcu_log_printf(IDCU_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define IDCU_LOG_INFO(fmt, ...)  idcu_log_printf(IDCU_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define IDCU_LOG_WARN(fmt, ...)  idcu_log_printf(IDCU_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define IDCU_LOG_ERROR(fmt, ...) idcu_log_printf(IDCU_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define IDCU_LOG_FATAL(fmt, ...) idcu_log_printf(IDCU_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
