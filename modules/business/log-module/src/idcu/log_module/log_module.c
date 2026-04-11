#include <idcu/log_module/log_module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int idcu_log_module_config_init(idcu_LogModuleConfig* config) {
    if (!config) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(config, 0, sizeof(idcu_LogModuleConfig));
    strncpy(config->name, "idcu-log-module", sizeof(config->name) - 1);
    strncpy(config->version, "1.0.0", sizeof(config->version) - 1);
    config->default_level = IDCU_LOG_LEVEL_INFO;
    config->enable_rotation = 1;
    config->max_file_size = 100 * 1024 * 1024;
    config->max_file_count = 10;
    config->enable_query = 1;
    config->enable_stats = 1;

    for (int i = 0; i < IDCU_LOG_OUTPUT_COUNT; i++) {
        config->outputs[i].type = (idcu_LogOutputType)i;
        config->outputs[i].min_level = IDCU_LOG_LEVEL_DEBUG;
        config->outputs[i].max_level = IDCU_LOG_LEVEL_FATAL;
        config->outputs[i].enabled = (i == IDCU_LOG_OUTPUT_CONSOLE) ? 1 : 0;
    }

    strncpy(config->outputs[IDCU_LOG_OUTPUT_CONSOLE].name, "console", sizeof(config->outputs[IDCU_LOG_OUTPUT_CONSOLE].name) - 1);
    strncpy(config->outputs[IDCU_LOG_OUTPUT_FILE].name, "file", sizeof(config->outputs[IDCU_LOG_OUTPUT_FILE].name) - 1);
    strncpy(config->outputs[IDCU_LOG_OUTPUT_FILE].path, "logs/app.log", sizeof(config->outputs[IDCU_LOG_OUTPUT_FILE].path) - 1);
    strncpy(config->outputs[IDCU_LOG_OUTPUT_MSGBUS].name, "msgbus", sizeof(config->outputs[IDCU_LOG_OUTPUT_MSGBUS].name) - 1);

    return IDCU_ERR_OK;
}

int idcu_log_module_init(idcu_LogModule* lm, const idcu_LogModuleConfig* config) {
    if (!lm || !config) {
        return IDCU_ERR_INVALID_ARG;
    }

    memset(lm, 0, sizeof(idcu_LogModule));
    memcpy(&lm->config, config, sizeof(idcu_LogModuleConfig));
    lm->initialized = 0;

    return IDCU_ERR_OK;
}

int idcu_log_module_start(idcu_LogModule* lm) {
    if (!lm) {
        return IDCU_ERR_INVALID_ARG;
    }

    idcu_LoggerConfig logger_config = {
        .level = lm->config.default_level,
        .output_path = lm->config.outputs[IDCU_LOG_OUTPUT_FILE].path,
        .enable_console = lm->config.outputs[IDCU_LOG_OUTPUT_CONSOLE].enabled,
        .enable_file = lm->config.outputs[IDCU_LOG_OUTPUT_FILE].enabled
    };

    int ret = idcu_logger_create(&logger_config, &lm->logger);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    lm->initialized = 1;

    return IDCU_ERR_OK;
}

int idcu_log_module_stop(idcu_LogModule* lm) {
    if (!lm) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (lm->logger) {
        idcu_logger_destroy(lm->logger);
        lm->logger = NULL;
    }

    lm->initialized = 0;

    return IDCU_ERR_OK;
}

void idcu_log_module_destroy(idcu_LogModule* lm) {
    if (!lm) {
        return;
    }

    if (lm->initialized) {
        idcu_log_module_stop(lm);
    }

    memset(lm, 0, sizeof(idcu_LogModule));
}

void idcu_log_module_log(idcu_LogModule* lm, idcu_LogLevel level, const char* fmt, ...) {
    if (!lm || !lm->logger) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    idcu_logger_vlog(lm->logger, level, fmt, args);
    va_end(args);

    if (lm->config.outputs[IDCU_LOG_OUTPUT_MSGBUS].enabled && lm->msgbus) {
    }
}

void idcu_log_module_debug(idcu_LogModule* lm, const char* fmt, ...) {
    if (!lm || !lm->logger) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    idcu_logger_vlog(lm->logger, IDCU_LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

void idcu_log_module_info(idcu_LogModule* lm, const char* fmt, ...) {
    if (!lm || !lm->logger) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    idcu_logger_vlog(lm->logger, IDCU_LOG_LEVEL_INFO, fmt, args);
    va_end(args);
}

void idcu_log_module_warn(idcu_LogModule* lm, const char* fmt, ...) {
    if (!lm || !lm->logger) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    idcu_logger_vlog(lm->logger, IDCU_LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void idcu_log_module_error(idcu_LogModule* lm, const char* fmt, ...) {
    if (!lm || !lm->logger) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    idcu_logger_vlog(lm->logger, IDCU_LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}

int idcu_log_module_set_level(idcu_LogModule* lm, idcu_LogLevel level) {
    if (!lm || !lm->logger) {
        return IDCU_ERR_INVALID_ARG;
    }

    lm->config.default_level = level;
    return idcu_logger_set_level(lm->logger, level);
}

idcu_LogLevel idcu_log_module_get_level(idcu_LogModule* lm) {
    if (!lm) {
        return IDCU_LOG_LEVEL_OFF;
    }
    return lm->config.default_level;
}

int idcu_log_module_enable_output(idcu_LogModule* lm, idcu_LogOutputType type, int enabled) {
    if (!lm || type >= IDCU_LOG_OUTPUT_COUNT) {
        return IDCU_ERR_INVALID_ARG;
    }

    lm->config.outputs[type].enabled = enabled ? 1 : 0;
    return IDCU_ERR_OK;
}

idcu_Logger* idcu_log_module_get_logger(idcu_LogModule* lm) {
    if (!lm) {
        return NULL;
    }
    return lm->logger;
}
