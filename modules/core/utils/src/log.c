#include "log.h"
#include "config.h"
#include "error_code.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

static FILE* log_file = NULL;
static idcu_LogLevel log_level = IDCU_LOG_INFO;

static const char* log_level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

int idcu_log_init(const char* filename, idcu_LogLevel level) {
    log_level = level;
    if (filename) {
        log_file = fopen(filename, "a");
        if (!log_file) return IDCU_ERR_GENERAL;
        setbuf(log_file, NULL);
    }
    return IDCU_ERR_SUCCESS;
}

void idcu_log_shutdown(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void idcu_log_set_level(idcu_LogLevel level) {
    if (level >= IDCU_LOG_DEBUG && level <= IDCU_LOG_FATAL) {
        log_level = level;
    }
}

idcu_LogLevel idcu_log_get_level(void) {
    return log_level;
}

int idcu_log_set_file(const char* filename) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    if (filename) {
        log_file = fopen(filename, "a");
        if (!log_file) return IDCU_ERR_GENERAL;
        setbuf(log_file, NULL);
    }
    return IDCU_ERR_SUCCESS;
}

void idcu_log_printf(idcu_LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (level < log_level) return;
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm);

    const char* level_name = log_level_names[level];

    char header[1024];
    snprintf(header, sizeof(header), "[%s] [%s] [%s:%d] ",
             time_buf, level_name, file, line);

    char content[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(content, sizeof(content), fmt, args);
    va_end(args);

    printf("%s%s\n", header, content);
    if (log_file) fprintf(log_file, "%s%s\n", header, content);
    if (level == IDCU_LOG_FATAL) exit(1);
}
