#ifndef IDCU_COMMON_ERROR_CODE_H
#define IDCU_COMMON_ERROR_CODE_H

#include <stddef.h>

typedef enum
{
    IDCU_ERR_OK      = 0,
    IDCU_ERR_SUCCESS = 0,

    IDCU_ERR_GENERAL           = -1,
    IDCU_ERR_INVALID_PARAM     = -2,
    IDCU_ERR_INVALID_ARG       = -2,
    IDCU_ERR_NO_MEMORY         = -3,
    IDCU_ERR_OUT_OF_MEM        = -3,
    IDCU_ERR_PERM_DENIED       = -4,
    IDCU_ERR_QUEUE_FULL        = -5,
    IDCU_ERR_QUEUE_EMPTY       = -6,
    IDCU_ERR_MODULE_NOT_FOUND  = -7,
    IDCU_ERR_MODULE_INIT       = -8,
    IDCU_ERR_CONFIG_LOAD       = -9,
    IDCU_ERR_MSG_SEND          = -10,
    IDCU_ERR_MSG_RECV          = -11,
    IDCU_ERR_CORO_CREATE       = -12,
    IDCU_ERR_SANDBOX_INIT      = -13,
    IDCU_ERR_MODULE_LOAD       = -14,
    IDCU_ERR_MODULE_INVALID    = -15,
    IDCU_ERR_NOT_FOUND         = -16,
    IDCU_ERR_ALREADY_EXISTS    = -17,
    IDCU_ERR_BUSY              = -18,
    IDCU_ERR_MODULE_RUN        = -19,
    IDCU_ERR_MODULE_STOP       = -20,
    IDCU_ERR_MODULE_CRASH      = -21,
    IDCU_ERR_DEPENDENCY        = -22,
    IDCU_ERR_CIRCULAR_DEP      = -23,
    IDCU_ERR_VERSION_MISMATCH  = -24,
    IDCU_ERR_NOT_INITIALIZED   = -25,
    IDCU_ERR_LOCK_FAILED       = -26,
    IDCU_ERR_UNLOCK_FAILED     = -27,
    IDCU_ERR_MSG_TIMEOUT       = -28,
    IDCU_ERR_MSG_DROPPED       = -29,
    IDCU_ERR_HEALTH_CHECK_FAIL = -30,
    IDCU_ERR_CONFIG_PARSE      = -31,
    IDCU_ERR_TIMEOUT           = -32,
    IDCU_ERR_CANCELLED         = -33,
    IDCU_ERR_CONFIG_INVALID    = -34,
    IDCU_ERR_DEADLOCK          = -35,
    IDCU_ERR_SANDBOX_ACCESS    = -36,
    IDCU_ERR_SANDBOX_VIOLATION = -37,
    IDCU_ERR_NETWORK_INIT      = -38,
    IDCU_ERR_NETWORK_CONNECT   = -39,
    IDCU_ERR_NETWORK_SEND      = -40,
    IDCU_ERR_NETWORK_RECV      = -41,
    IDCU_ERR_NETWORK_TIMEOUT   = -42,
    IDCU_ERR_METRICS_COLLECT   = -43,
    IDCU_ERR_FILE_OPEN         = -44,
    IDCU_ERR_FILE_READ         = -45,
    IDCU_ERR_FILE_WRITE        = -46,
    IDCU_ERR_FILE_DELETE       = -47,
    IDCU_ERR_CORO_SCHEDULE     = -48,
    IDCU_ERR_CORO_YIELD        = -49,
    IDCU_ERR_BUFFER_TOO_SMALL  = -50,
    IDCU_ERR_VERSION_TOO_OLD   = -51,
    IDCU_ERR_VERSION_TOO_NEW   = -52,
    IDCU_ERR_VERSION_INCOMPATIBLE = -53,
    IDCU_ERR_MEMORY            = -54,
    IDCU_ERR_LIMIT_EXCEEDED    = -55,
    IDCU_ERR_IO_ERROR          = -56,
    IDCU_ERR_FILE_ERROR        = -56,
    IDCU_ERR_SECURITY_ERROR    = -57,
    IDCU_ERR_RESOURCE_EXHAUSTED = -58,
    IDCU_ERR_VERIFICATION_FAILED = -59
} idcu_ErrorCode;

#define IDCU_SUCCESS                    IDCU_ERR_SUCCESS
#define IDCU_ERROR_INVALID_PARAM        IDCU_ERR_INVALID_PARAM
#define IDCU_ERROR_MEMORY               IDCU_ERR_MEMORY
#define IDCU_ERROR_BUFFER_TOO_SMALL     IDCU_ERR_BUFFER_TOO_SMALL
#define IDCU_ERROR_VERSION_TOO_OLD      IDCU_ERR_VERSION_TOO_OLD
#define IDCU_ERROR_VERSION_TOO_NEW      IDCU_ERR_VERSION_TOO_NEW
#define IDCU_ERROR_VERSION_INCOMPATIBLE IDCU_ERR_VERSION_INCOMPATIBLE

#define IDCU_ERROR_CONTEXT_MAX_LEN 256

typedef struct
{
    int         error_code;
    char        context[IDCU_ERROR_CONTEXT_MAX_LEN];
    const char* file;
    int         line;
} idcu_ErrorInfo;

const char* idcu_err_to_str(int err_code);

static inline int idcu_err_is_ok(int err_code)
{
    return err_code == IDCU_ERR_OK;
}

void idcu_err_set_last_error(int err_code, const char* context, const char* file, int line);
const idcu_ErrorInfo* idcu_err_get_last_error(void);
void idcu_err_clear_last_error(void);

#define IDCU_ERR_SET(code, ctx) idcu_err_set_last_error((code), (ctx), __FILE__, __LINE__)

#endif
