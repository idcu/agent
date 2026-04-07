#ifndef IDCU_SDK_MODULE_ERROR_H
#define IDCU_SDK_MODULE_ERROR_H

#include "idcu/common/error_code.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IDCU_MOD_ERR_BASE = 1000,
    IDCU_MOD_ERR_INVALID_STATE = IDCU_MOD_ERR_BASE + 1,
    IDCU_MOD_ERR_DEPENDENCY_MISSING = IDCU_MOD_ERR_BASE + 2,
    IDCU_MOD_ERR_DEPENDENCY_VERSION_MISMATCH = IDCU_MOD_ERR_BASE + 3,
    IDCU_MOD_ERR_DEPENDENCY_CYCLE = IDCU_MOD_ERR_BASE + 4,
    IDCU_MOD_ERR_RESOURCE_EXHAUSTED = IDCU_MOD_ERR_BASE + 5,
    IDCU_MOD_ERR_RESOURCE_LIMIT_EXCEEDED = IDCU_MOD_ERR_BASE + 6,
    IDCU_MOD_ERR_INITIALIZATION_FAILED = IDCU_MOD_ERR_BASE + 7,
    IDCU_MOD_ERR_START_FAILED = IDCU_MOD_ERR_BASE + 8,
    IDCU_MOD_ERR_STOP_FAILED = IDCU_MOD_ERR_BASE + 9,
    IDCU_MOD_ERR_COMMUNICATION_FAILED = IDCU_MOD_ERR_BASE + 10,
    IDCU_MOD_ERR_TIMEOUT = IDCU_MOD_ERR_BASE + 11,
    IDCU_MOD_ERR_PERMISSION_DENIED = IDCU_MOD_ERR_BASE + 12,
    IDCU_MOD_ERR_INVALID_CONFIG = IDCU_MOD_ERR_BASE + 13,
    IDCU_MOD_ERR_MODULE_NOT_FOUND = IDCU_MOD_ERR_BASE + 14,
    IDCU_MOD_ERR_MODULE_ALREADY_LOADED = IDCU_MOD_ERR_BASE + 15,
    IDCU_MOD_ERR_SANDBOX_VIOLATION = IDCU_MOD_ERR_BASE + 16,
    IDCU_MOD_ERR_LAST = IDCU_MOD_ERR_BASE + 99
} idcu_ModuleErrorCode;

typedef struct {
    int error_code;
    const char* error_message;
    const char* module_name;
    const char* file;
    int line;
    int64_t timestamp;
    void* user_data;
} idcu_ModuleError;

typedef struct idcu_module_error_context idcu_ModuleErrorContext;

typedef void (*idcu_ModuleErrorHandler)(idcu_ModuleErrorContext* ctx,
                                        const idcu_ModuleError* error,
                                        void* user_data);

idcu_ModuleErrorContext* idcu_module_error_create_context(void);
void idcu_module_error_destroy_context(idcu_ModuleErrorContext* ctx);

void* idcu_module_error_get_user_data(idcu_ModuleErrorContext* ctx);
void idcu_module_error_set_user_data(idcu_ModuleErrorContext* ctx, void* user_data);

void idcu_module_error_set_handler(idcu_ModuleErrorContext* ctx,
                                    idcu_ModuleErrorHandler handler,
                                    void* user_data);

int idcu_module_error_raise(idcu_ModuleErrorContext* ctx,
                            int error_code,
                            const char* error_message,
                            const char* module_name,
                            const char* file,
                            int line);

int idcu_module_error_raise_with_data(idcu_ModuleErrorContext* ctx,
                                       int error_code,
                                       const char* error_message,
                                       const char* module_name,
                                       const char* file,
                                       int line,
                                       void* user_data);

const char* idcu_module_error_get_message(int error_code);

void idcu_module_error_clear(idcu_ModuleErrorContext* ctx);

int idcu_module_error_get_last(idcu_ModuleErrorContext* ctx, idcu_ModuleError* out_error);

#define IDCU_MOD_RAISE(ctx, code, msg) \
    idcu_module_error_raise(ctx, code, msg, __func__, __FILE__, __LINE__)

#define IDCU_MOD_RAISE_WITH_DATA(ctx, code, msg, data) \
    idcu_module_error_raise_with_data(ctx, code, msg, __func__, __FILE__, __LINE__, data)

#ifdef __cplusplus
}
#endif

#endif
