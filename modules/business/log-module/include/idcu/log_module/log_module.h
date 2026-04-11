#ifndef IDCU_LOG_MODULE_LOG_MODULE_H
#define IDCU_LOG_MODULE_LOG_MODULE_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/log/log.h>
#include <idcu/msgbus/msgbus.h>
#include <idcu/sdk/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_LOG_OUTPUT_CONSOLE = 0,
    IDCU_LOG_OUTPUT_FILE,
    IDCU_LOG_OUTPUT_MSGBUS,
    IDCU_LOG_OUTPUT_COUNT
} idcu_LogOutputType;

typedef struct {
    char name[128];
    idcu_LogOutputType type;
    idcu_LogLevel min_level;
    idcu_LogLevel max_level;
    char path[1024];
    int enabled;
} idcu_LogOutput;

typedef struct {
    char name[128];
    char version[64];
    idcu_LogLevel default_level;
    idcu_LogOutput outputs[IDCU_LOG_OUTPUT_COUNT];
    int enable_rotation;
    size_t max_file_size;
    int max_file_count;
    int enable_query;
    int enable_stats;
} idcu_LogModuleConfig;

typedef struct {
    idcu_LogModuleConfig config;
    idcu_Logger* logger;
    idcu_MsgBus* msgbus;
    int initialized;
    void* user_data;
} idcu_LogModule;

int idcu_log_module_config_init(idcu_LogModuleConfig* config);

int idcu_log_module_init(idcu_LogModule* lm, const idcu_LogModuleConfig* config);

int idcu_log_module_start(idcu_LogModule* lm);

int idcu_log_module_stop(idcu_LogModule* lm);

void idcu_log_module_destroy(idcu_LogModule* lm);

void idcu_log_module_log(idcu_LogModule* lm, idcu_LogLevel level, const char* fmt, ...);

void idcu_log_module_debug(idcu_LogModule* lm, const char* fmt, ...);

void idcu_log_module_info(idcu_LogModule* lm, const char* fmt, ...);

void idcu_log_module_warn(idcu_LogModule* lm, const char* fmt, ...);

void idcu_log_module_error(idcu_LogModule* lm, const char* fmt, ...);

int idcu_log_module_set_level(idcu_LogModule* lm, idcu_LogLevel level);

idcu_LogLevel idcu_log_module_get_level(idcu_LogModule* lm);

int idcu_log_module_enable_output(idcu_LogModule* lm, idcu_LogOutputType type, int enabled);

idcu_Logger* idcu_log_module_get_logger(idcu_LogModule* lm);

#ifdef __cplusplus
}
#endif

#endif
