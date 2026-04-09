#ifndef IDCU_LOG_TYPES_H
#define IDCU_LOG_TYPES_H

#include <idcu/common/config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_LOG_DEBUG = 0,
    IDCU_LOG_INFO = 1,
    IDCU_LOG_WARN = 2,
    IDCU_LOG_ERROR = 3,
    IDCU_LOG_FATAL = 4
} idcu_LogLevel;

typedef enum {
    IDCU_LOG_OUTPUT_CONSOLE = 1 << 0,
    IDCU_LOG_OUTPUT_FILE = 1 << 1
} idcu_LogOutput;

typedef struct {
    char           filename[256];
    idcu_LogLevel  level;
    idcu_LogOutput  output;
} idcu_LogConfig;

#ifdef __cplusplus
}
#endif

#endif
