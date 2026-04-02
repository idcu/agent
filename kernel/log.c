#include "log.h"
#include "config.h"
#include "error_code.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

static FILE* log_file = NULL;
static LogLevel log_level = LOG_INFO;

const char* err_to_str(int err_code) {
    switch (err_code) {
        case ERR_SUCCESS: return "Success";
        case ERR_GENERAL: return "General error";
        case ERR_INVALID_PARAM: return "Invalid parameter";
        case ERR_OUT_OF_MEM: return "Out of memory";
        case ERR_PERM_DENIED: return "Permission denied";
        case ERR_QUEUE_FULL: return "Queue full";
        case ERR_QUEUE_EMPTY: return "Queue empty";
        case ERR_MODULE_NOT_FOUND: return "Module not found";
        case ERR_MODULE_INIT: return "Module init failed";
        case ERR_CONFIG_LOAD: return "Config load failed";
        case ERR_MSG_SEND: return "Message send failed";
        case ERR_MSG_RECV: return "Message recv failed";
        case ERR_CORO_CREATE: return "Coroutine create failed";
        case ERR_SANDBOX_INIT: return "Sandbox init failed";
        default: return "Unknown error";
    }
}

int log_init(const char* filename, LogLevel level) {
    log_level = level;
    if (filename) {
        log_file = fopen(filename, "a");
        if (!log_file) return ERR_GENERAL;
        setbuf(log_file, NULL);
    }
    return ERR_SUCCESS;
}

void log_printf(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (level < log_level) return;
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm);

    char header[1024];
    snprintf(header, sizeof(header), "[%s] [%s] [%s:%d] ",
             time_buf,
             (level == 0 ? "DEBUG" : level == 1 ? "INFO" : level == 2 ? "WARN" : level == 3 ? "ERROR" : "FATAL"),
             file, line);

    char content[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(content, sizeof(content), fmt, args);
    va_end(args);

    printf("%s%s\n", header, content);
    if (log_file) fprintf(log_file, "%s%s\n", header, content);
    if (level == LOG_FATAL) exit(1);
}