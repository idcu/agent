#include "idcu/common/error_code.h"
#include <string.h>

static idcu_ErrorInfo g_last_error = {0};

const char* idcu_err_to_str(int err_code)
{
    switch (err_code) {
    case IDCU_ERR_OK: return "Success";
    case IDCU_ERR_GENERAL: return "General error";
    case IDCU_ERR_INVALID_PARAM: return "Invalid parameter";
    case IDCU_ERR_NO_MEMORY: return "Out of memory";
    case IDCU_ERR_PERM_DENIED: return "Permission denied";
    case IDCU_ERR_QUEUE_FULL: return "Queue full";
    case IDCU_ERR_QUEUE_EMPTY: return "Queue empty";
    case IDCU_ERR_MODULE_NOT_FOUND: return "Module not found";
    case IDCU_ERR_MODULE_INIT: return "Module initialization failed";
    case IDCU_ERR_CONFIG_LOAD: return "Config load failed";
    case IDCU_ERR_MSG_SEND: return "Message send failed";
    case IDCU_ERR_MSG_RECV: return "Message receive failed";
    case IDCU_ERR_CORO_CREATE: return "Coroutine create failed";
    case IDCU_ERR_SANDBOX_INIT: return "Sandbox init failed";
    case IDCU_ERR_MODULE_LOAD: return "Module load failed";
    case IDCU_ERR_MODULE_INVALID: return "Invalid module";
    case IDCU_ERR_NOT_FOUND: return "Not found";
    case IDCU_ERR_ALREADY_EXISTS: return "Already exists";
    case IDCU_ERR_BUSY: return "Resource busy";
    case IDCU_ERR_MODULE_RUN: return "Module run failed";
    case IDCU_ERR_MODULE_STOP: return "Module stop failed";
    case IDCU_ERR_MODULE_CRASH: return "Module crashed";
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
    case IDCU_ERR_CONFIG_INVALID: return "Invalid config";
    case IDCU_ERR_DEADLOCK: return "Deadlock detected";
    case IDCU_ERR_SANDBOX_ACCESS: return "Sandbox access denied";
    case IDCU_ERR_SANDBOX_VIOLATION: return "Sandbox violation";
    case IDCU_ERR_NETWORK_INIT: return "Network init failed";
    case IDCU_ERR_NETWORK_CONNECT: return "Network connect failed";
    case IDCU_ERR_NETWORK_SEND: return "Network send failed";
    case IDCU_ERR_NETWORK_RECV: return "Network receive failed";
    case IDCU_ERR_NETWORK_TIMEOUT: return "Network timeout";
    case IDCU_ERR_METRICS_COLLECT: return "Metrics collect failed";
    case IDCU_ERR_FILE_OPEN: return "File open failed";
    case IDCU_ERR_FILE_READ: return "File read failed";
    case IDCU_ERR_FILE_WRITE: return "File write failed";
    case IDCU_ERR_FILE_DELETE: return "File delete failed";
    case IDCU_ERR_CORO_SCHEDULE: return "Coroutine schedule failed";
    case IDCU_ERR_CORO_YIELD: return "Coroutine yield failed";
    case IDCU_ERR_BUFFER_TOO_SMALL: return "Buffer too small";
    case IDCU_ERR_VERSION_TOO_OLD: return "Version too old";
    case IDCU_ERR_VERSION_TOO_NEW: return "Version too new";
    case IDCU_ERR_VERSION_INCOMPATIBLE: return "Version incompatible";
    case IDCU_ERR_MEMORY: return "Memory allocation failed";
    case IDCU_ERR_LIMIT_EXCEEDED: return "Limit exceeded";
    case IDCU_ERR_IO_ERROR: return "IO error";
    case IDCU_ERR_SECURITY_ERROR: return "Security error";
    case IDCU_ERR_RESOURCE_EXHAUSTED: return "Resource exhausted";
    case IDCU_ERR_VERIFICATION_FAILED: return "Verification failed";
    default: return "Unknown error";
    }
}

void idcu_err_set_last_error(int err_code, const char* context, const char* file, int line)
{
    g_last_error.error_code = err_code;
    g_last_error.file = file;
    g_last_error.line = line;

    if (context) {
        strncpy(g_last_error.context, context, IDCU_ERROR_CONTEXT_MAX_LEN - 1);
        g_last_error.context[IDCU_ERROR_CONTEXT_MAX_LEN - 1] = '\0';
    } else {
        g_last_error.context[0] = '\0';
    }
}

const idcu_ErrorInfo* idcu_err_get_last_error(void)
{
    return &g_last_error;
}

void idcu_err_clear_last_error(void)
{
    memset(&g_last_error, 0, sizeof(g_last_error));
}
