
#include "common/error_code.h"

const char* idcu_err_to_str(int err_code) {
    switch (err_code) {
        // 成功状态码
        case IDCU_ERR_OK:
            return "Success";

        // 通用错误类
        case IDCU_ERR_GENERAL:
            return "General error";
        case IDCU_ERR_INVALID_PARAM:
            return "Invalid parameter";
        case IDCU_ERR_NO_MEMORY:
            return "Out of memory";
        case IDCU_ERR_PERM_DENIED:
            return "Permission denied";
        case IDCU_ERR_QUEUE_FULL:
            return "Queue full";
        case IDCU_ERR_QUEUE_EMPTY:
            return "Queue empty";
        case IDCU_ERR_MODULE_NOT_FOUND:
            return "Module not found";
        case IDCU_ERR_MODULE_INIT:
            return "Module initialization failed";
        case IDCU_ERR_CONFIG_LOAD:
            return "Config load failed";
        case IDCU_ERR_MSG_SEND:
            return "Message send failed";
        case IDCU_ERR_MSG_RECV:
            return "Message receive failed";
        case IDCU_ERR_CORO_CREATE:
            return "Coroutine create failed";
        case IDCU_ERR_SANDBOX_INIT:
            return "Sandbox init failed";
        case IDCU_ERR_MODULE_LOAD:
            return "Module load failed";
        case IDCU_ERR_MODULE_INVALID:
            return "Invalid module";
        case IDCU_ERR_NOT_FOUND:
            return "Not found";
        case IDCU_ERR_ALREADY_EXISTS:
            return "Already exists";
        case IDCU_ERR_BUSY:
            return "Resource busy";
        case IDCU_ERR_MODULE_RUN:
            return "Module run failed";
        case IDCU_ERR_MODULE_STOP:
            return "Module stop failed";
        case IDCU_ERR_MODULE_CRASH:
            return "Module crashed";
        case IDCU_ERR_DEPENDENCY:
            return "Dependency error";
        case IDCU_ERR_CIRCULAR_DEP:
            return "Circular dependency";
        case IDCU_ERR_VERSION_MISMATCH:
            return "Version mismatch";
        case IDCU_ERR_NOT_INITIALIZED:
            return "Not initialized";
        case IDCU_ERR_LOCK_FAILED:
            return "Lock failed";
        case IDCU_ERR_UNLOCK_FAILED:
            return "Unlock failed";
        case IDCU_ERR_MSG_TIMEOUT:
            return "Message timeout";
        case IDCU_ERR_MSG_DROPPED:
            return "Message dropped";
        case IDCU_ERR_HEALTH_CHECK_FAIL:
            return "Health check failed";
        case IDCU_ERR_CONFIG_PARSE:
            return "Config parse error";
        case IDCU_ERR_TIMEOUT:
            return "Timeout";
        case IDCU_ERR_CANCELLED:
            return "Cancelled";
        case IDCU_ERR_CONFIG_INVALID:
            return "Invalid config";
        case IDCU_ERR_DEADLOCK:
            return "Deadlock detected";
        case IDCU_ERR_SANDBOX_ACCESS:
            return "Sandbox access denied";
        case IDCU_ERR_SANDBOX_VIOLATION:
            return "Sandbox violation";
        case IDCU_ERR_NETWORK_INIT:
            return "Network init failed";
        case IDCU_ERR_NETWORK_CONNECT:
            return "Network connect failed";
        case IDCU_ERR_NETWORK_SEND:
            return "Network send failed";
        case IDCU_ERR_NETWORK_RECV:
            return "Network receive failed";
        case IDCU_ERR_NETWORK_TIMEOUT:
            return "Network timeout";
        case IDCU_ERR_METRICS_COLLECT:
            return "Metrics collect failed";
        case IDCU_ERR_FILE_OPEN:
            return "File open failed";
        case IDCU_ERR_FILE_READ:
            return "File read failed";
        case IDCU_ERR_FILE_WRITE:
            return "File write failed";
        case IDCU_ERR_FILE_DELETE:
            return "File delete failed";
        case IDCU_ERR_CORO_SCHEDULE:
            return "Coroutine schedule failed";
        case IDCU_ERR_CORO_YIELD:
            return "Coroutine yield failed";

        default:
            return "Unknown error";
    }
}

