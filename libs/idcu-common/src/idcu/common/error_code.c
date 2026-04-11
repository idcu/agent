#include "idcu/common/error_code.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
    case IDCU_ERR_UNKNOWN: return "Unknown error";
    case IDCU_ERR_INVALID_STATE: return "Invalid state";
    case IDCU_ERR_OUT_OF_RANGE: return "Out of range";
    default: return "Unknown error";
    }
}

void idcu_err_set_last_error(int err_code, const char* context, const char* file, int line)
{
    g_last_error.error_code = err_code;
    g_last_error.file = file;
    g_last_error.line = line;
    g_last_error.cause = NULL;
    g_last_error.stacktrace_frame_count = 0;

    if (context) {
        strncpy(g_last_error.context, context, IDCU_ERROR_CONTEXT_MAX_LEN - 1);
        g_last_error.context[IDCU_ERROR_CONTEXT_MAX_LEN - 1] = '\0';
    } else {
        g_last_error.context[0] = '\0';
    }

#ifdef IDCU_DEBUG
    idcu_err_capture_stacktrace(&g_last_error);
#endif
}

void idcu_err_set_last_error_with_cause(int err_code, const char* context, 
                                         const char* file, int line, idcu_ErrorInfo* cause)
{
    idcu_err_set_last_error(err_code, context, file, line);
    
    if (cause) {
        g_last_error.cause = idcu_err_clone_error(cause);
    }
}

const idcu_ErrorInfo* idcu_err_get_last_error(void)
{
    return &g_last_error;
}

void idcu_err_clear_last_error(void)
{
    idcu_err_free_error(g_last_error.cause);
    memset(&g_last_error, 0, sizeof(g_last_error));
}

idcu_ErrorInfo* idcu_err_clone_error(const idcu_ErrorInfo* error)
{
    if (!error) {
        return NULL;
    }

    idcu_ErrorInfo* cloned = (idcu_ErrorInfo*)malloc(sizeof(idcu_ErrorInfo));
    if (!cloned) {
        return NULL;
    }

    *cloned = *error;
    cloned->cause = idcu_err_clone_error(error->cause);
    
    return cloned;
}

void idcu_err_free_error(idcu_ErrorInfo* error)
{
    if (!error) {
        return;
    }

    idcu_err_free_error(error->cause);
    free(error);
}

bool idcu_err_has_cause(const idcu_ErrorInfo* error)
{
    return error && error->cause != NULL;
}

const idcu_ErrorInfo* idcu_err_get_cause(const idcu_ErrorInfo* error)
{
    return error ? error->cause : NULL;
}

int idcu_err_format_error(const idcu_ErrorInfo* error, char* buffer, size_t buffer_size)
{
    if (!error || !buffer || buffer_size == 0) {
        return 0;
    }

    int written = snprintf(buffer, buffer_size, "[%d] %s", 
                          error->error_code, idcu_err_to_str(error->error_code));
    
    if (written > 0 && (size_t)written < buffer_size && error->context[0] != '\0') {
        written += snprintf(buffer + written, buffer_size - written, 
                            " - %s", error->context);
    }

    if (written > 0 && (size_t)written < buffer_size && error->file) {
        written += snprintf(buffer + written, buffer_size - written, 
                            " at %s:%d", error->file, error->line);
    }

    return written;
}

int idcu_err_format_error_chain(const idcu_ErrorInfo* error, char* buffer, size_t buffer_size)
{
    if (!error || !buffer || buffer_size == 0) {
        return 0;
    }

    size_t total_written = 0;
    int depth = 0;
    const idcu_ErrorInfo* current = error;

    while (current && depth < IDCU_ERROR_CHAIN_MAX_DEPTH && total_written < buffer_size - 1) {
        if (depth > 0) {
            if (total_written < buffer_size - 1) {
                buffer[total_written++] = '\n';
            }
        }

        int written = idcu_err_format_error(current, buffer + total_written, 
                                        buffer_size - total_written);
        
        if (written <= 0) break;
        total_written += written;

        current = current->cause;
        depth++;
    }

    buffer[total_written] = '\0';
    return (int)total_written;
}

#ifdef IDCU_DEBUG

#if defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#elif defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

void idcu_err_capture_stacktrace(idcu_ErrorInfo* error)
{
    if (!error) {
        return;
    }

#if defined(__linux__) || defined(__APPLE__)
    void* frames[IDCU_ERROR_STACKTRACE_MAX_FRAMES];
    int num_frames = backtrace(frames, IDCU_ERROR_STACKTRACE_MAX_FRAMES);
    
    if (num_frames > 0) {
        error->stacktrace_frame_count = num_frames;
        memcpy(error->stacktrace_frames, frames, num_frames * sizeof(void*));
    }
#elif defined(_WIN32)
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    
    void* frames[IDCU_ERROR_STACKTRACE_MAX_FRAMES];
    WORD num_frames = CaptureStackBackTrace(0, IDCU_ERROR_STACKTRACE_MAX_FRAMES, frames, NULL);
    
    if (num_frames > 0) {
        error->stacktrace_frame_count = num_frames;
        memcpy(error->stacktrace_frames, frames, num_frames * sizeof(void*));
    }
#endif
}

int idcu_err_format_stacktrace(const idcu_ErrorInfo* error, char* buffer, size_t buffer_size)
{
    if (!error || !buffer || buffer_size == 0 || error->stacktrace_frame_count == 0) {
        return 0;
    }

    size_t total_written = 0;
    
#if defined(__linux__) || defined(__APPLE__)
    char** symbols = backtrace_symbols(error->stacktrace_frames, error->stacktrace_frame_count);
    if (!symbols) {
        return 0;
    }

    for (int i = 0; i < error->stacktrace_frame_count && total_written < buffer_size - 1; i++) {
        int written = snprintf(buffer + total_written, buffer_size - total_written, 
                              "%s  #%d: %s\n", i > 0 ? "" : "Stacktrace:\n", i, symbols[i]);
        
        if (written <= 0) break;
        total_written += written;
    }

    free(symbols);
#elif defined(_WIN32)
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    
    for (int i = 0; i < error->stacktrace_frame_count && total_written < buffer_size - 1; i++) {
        char symbol_buffer[sizeof(SYMBOL_INFO) + 256];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)symbol_buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;
        
        char frame_info[256] = {0};
        if (SymFromAddr(process, (DWORD64)error->stacktrace_frames[i], 0, symbol)) {
            snprintf(frame_info, sizeof(frame_info), "%s", symbol->Name);
        } else {
            snprintf(frame_info, sizeof(frame_info), "0x%p", error->stacktrace_frames[i]);
        }
        
        int written = snprintf(buffer + total_written, buffer_size - total_written, 
                              "%s  #%d: %s\n", i > 0 ? "" : "Stacktrace:\n", i, frame_info);
        
        if (written <= 0) break;
        total_written += written;
    }
#endif

    buffer[total_written] = '\0';
    return (int)total_written;
}

#endif
