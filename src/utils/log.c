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
        case IDCU_ERR_SUCCESS: return "Success";
        case IDCU_ERR_GENERAL: return "General error";
        case IDCU_ERR_INVALID_PARAM: return "Invalid parameter";
        case IDCU_ERR_NO_MEMORY: return "Out of memory";
        case IDCU_ERR_PERM_DENIED: return "Permission denied";
        case IDCU_ERR_QUEUE_FULL: return "Queue full";
        case IDCU_ERR_QUEUE_EMPTY: return "Queue empty";
        case IDCU_ERR_MODULE_NOT_FOUND: return "Module not found";
        case IDCU_ERR_MODULE_INIT: return "Module init failed";
        case IDCU_ERR_CONFIG_LOAD: return "Config load failed";
        case IDCU_ERR_MSG_SEND: return "Message send failed";
        case IDCU_ERR_MSG_RECV: return "Message recv failed";
        case IDCU_ERR_CORO_CREATE: return "Coroutine create failed";
        case IDCU_ERR_SANDBOX_INIT: return "Sandbox init failed";
        case IDCU_ERR_MODULE_LOAD: return "Module load failed";
        case IDCU_ERR_MODULE_INVALID: return "Module invalid";
        case IDCU_ERR_NOT_FOUND: return "Not found";
        case IDCU_ERR_ALREADY_EXISTS: return "Already exists";
        case IDCU_ERR_BUSY: return "Busy";
        case IDCU_ERR_MODULE_RUN: return "Module run failed";
        case IDCU_ERR_MODULE_STOP: return "Module stop failed";
        case IDCU_ERR_MODULE_CRASH: return "Module crash";
        case IDCU_ERR_DEPENDENCY: return "Dependency error";
        case IDCU_ERR_CIRCULAR_DEP: return "Circular dependency";
        case IDCU_ERR_VERSION_MISMATCH: return "Version mismatch";
        case IDCU_ERR_NOT_INITIALIZED: return "Not initialized";
        case IDCU_ERR_LOCK_FAILED: return "Lock failed";
        case IDCU_ERR_UNLOCK_FAILED: return "Unlock failed";
        case IDCU_ERR_MSG_TIMEOUT: return "Message timeout";
        case IDCU_ERR_MSG_DROPPED: return "Message dropped";
        case IDCU_ERR_HEALTH_CHECK_FAIL: return "Health check failed";
        case IDCU_ERR_CONFIG_PARSE: return "Config parse error";
        case IDCU_ERR_TIMEOUT: return "Timeout";
        case IDCU_ERR_CANCELLED: return "Cancelled";
        case IDCU_ERR_CONFIG_INVALID: return "Config invalid";
        case IDCU_ERR_DEADLOCK: return "Deadlock";
        case IDCU_ERR_SANDBOX_ACCESS: return "Sandbox access denied";
        case IDCU_ERR_SANDBOX_VIOLATION: return "Sandbox violation";
        case IDCU_ERR_NETWORK_INIT: return "Network init failed";
        case IDCU_ERR_NETWORK_CONNECT: return "Network connect failed";
        case IDCU_ERR_NETWORK_SEND: return "Network send failed";
        case IDCU_ERR_NETWORK_RECV: return "Network recv failed";
        case IDCU_ERR_NETWORK_TIMEOUT: return "Network timeout";
        case IDCU_ERR_METRICS_COLLECT: return "Metrics collect failed";
        case IDCU_ERR_FILE_OPEN: return "File open failed";
        case IDCU_ERR_FILE_READ: return "File read failed";
        case IDCU_ERR_FILE_WRITE: return "File write failed";
        case IDCU_ERR_FILE_DELETE: return "File delete failed";
        case IDCU_ERR_CORO_SCHEDULE: return "Coroutine schedule failed";
        case IDCU_ERR_CORO_YIELD: return "Coroutine yield failed";
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
