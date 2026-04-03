#include "utils/log.h"
#include "common/config.h"
#include "common/error_code.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

static FILE* log_file = NULL;
static idcu_LogLevel log_level = IDCU_LOG_INFO;

static const char* log_level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

const char* idcu_err_to_str(int err_code) {
    switch (err_code) {
        case 0: return "Success";
        case -1: return "General error";
        case -2: return "Invalid parameter";
        case -3: return "Out of memory";
        case -4: return "Permission denied";
        case -5: return "Queue full";
        case -6: return "Queue empty";
        case -7: return "Module not found";
        case -8: return "Module init failed";
        case -9: return "Config load failed";
        case -10: return "Message send failed";
        case -11: return "Message recv failed";
        case -12: return "Coroutine create failed";
        case -13: return "Sandbox init failed";
        case -14: return "Module load failed";
        case -15: return "Module invalid";
        case -16: return "Not found";
        case -17: return "Already exists";
        case -18: return "Busy";
        case -19: return "Module run failed";
        case -20: return "Module stop failed";
        case -21: return "Module crash";
        case -22: return "Dependency error";
        case -23: return "Circular dependency";
        case -24: return "Version mismatch";
        case -25: return "Not initialized";
        case -26: return "Lock failed";
        case -27: return "Unlock failed";
        case -28: return "Message timeout";
        case -29: return "Message dropped";
        case -30: return "Health check failed";
        case -31: return "Config parse error";
        default: return "Unknown error";
    }
}

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
