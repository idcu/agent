#include <idcu/log/log.h>
#include <idcu/common/lock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static struct {
    idcu_LogConfig config;
    FILE* file;
    idcu_Mutex mutex;
    int initialized;
} g_logger = {0};

static const char* level_to_str(idcu_LogLevel level) {
    switch (level) {
        case IDCU_LOG_DEBUG: return "DEBUG";
        case IDCU_LOG_INFO:  return "INFO ";
        case IDCU_LOG_WARN:  return "WARN ";
        case IDCU_LOG_ERROR: return "ERROR";
        case IDCU_LOG_FATAL: return "FATAL";
        default: return "UNKN ";
    }
}

static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm tm_info;

#ifdef _WIN32
    localtime_s(&tm_info, &now);
#else
    localtime_r(&now, &tm_info);
#endif

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

#ifdef _WIN32
static WORD get_console_color(idcu_LogLevel level) {
    switch (level) {
        case IDCU_LOG_DEBUG: return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case IDCU_LOG_INFO:  return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case IDCU_LOG_WARN:  return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case IDCU_LOG_ERROR: return FOREGROUND_RED | FOREGROUND_INTENSITY;
        case IDCU_LOG_FATAL: return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        default: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}
#endif

int idcu_log_init(const char* filename, idcu_LogLevel level) {
    idcu_LogConfig config;
    memset(&config, 0, sizeof(config));
    config.level = level;
    config.output = IDCU_LOG_OUTPUT_CONSOLE;

    if (filename) {
        strncpy(config.filename, filename, sizeof(config.filename) - 1);
        config.output |= IDCU_LOG_OUTPUT_FILE;
    }

    return idcu_log_init_with_config(&config);
}

int idcu_log_init_with_config(const idcu_LogConfig* config) {
    if (!config) {
        return IDCU_ERR_INVALID_ARG;
    }

    if (g_logger.initialized) {
        idcu_log_shutdown();
    }

    memcpy(&g_logger.config, config, sizeof(g_logger.config));

    int ret = idcu_mutex_init(&g_logger.mutex);
    if (ret != IDCU_ERR_OK) {
        return ret;
    }

    if (config->output & IDCU_LOG_OUTPUT_FILE && strlen(config->filename) > 0) {
        g_logger.file = fopen(config->filename, "a");
        if (!g_logger.file) {
            idcu_mutex_destroy(&g_logger.mutex);
            return IDCU_ERR_FILE_OPEN;
        }
    }

    g_logger.initialized = 1;
    return IDCU_ERR_OK;
}

void idcu_log_shutdown(void) {
    if (!g_logger.initialized) {
        return;
    }

    if (g_logger.file) {
        fclose(g_logger.file);
        g_logger.file = NULL;
    }

    idcu_mutex_destroy(&g_logger.mutex);

    memset(&g_logger, 0, sizeof(g_logger));
}

void idcu_log_set_level(idcu_LogLevel level) {
    if (g_logger.initialized) {
        g_logger.config.level = level;
    }
}

void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (!g_logger.initialized) {
        return;
    }

    if (level < g_logger.config.level) {
        return;
    }

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    char message[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    char buffer[8192];
    snprintf(buffer, sizeof(buffer), "[%s] [%s] %s:%d - %s\n",
             timestamp, level_to_str(level), file, line, message);

    idcu_mutex_lock(&g_logger.mutex);

    if (g_logger.config.output & IDCU_LOG_OUTPUT_CONSOLE) {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        WORD savedAttributes;
        
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        savedAttributes = consoleInfo.wAttributes;
        
        SetConsoleTextAttribute(hConsole, get_console_color(level));
        fprintf(stdout, "%s", buffer);
        SetConsoleTextAttribute(hConsole, savedAttributes);
#else
        fprintf(stdout, "%s", buffer);
#endif
        fflush(stdout);
    }

    if (g_logger.config.output & IDCU_LOG_OUTPUT_FILE && g_logger.file) {
        fprintf(g_logger.file, "%s", buffer);
        fflush(g_logger.file);
    }

    idcu_mutex_unlock(&g_logger.mutex);
}
