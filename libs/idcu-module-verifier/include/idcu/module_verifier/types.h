#ifndef IDCU_MODULE_VERIFIER_TYPES_H
#define IDCU_MODULE_VERIFIER_TYPES_H

#include <idcu/common/config.h>
#include <idcu/common/lock.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDCU_MODULE_VERIFIER_MAX_ITEMS 1024

typedef enum {
    IDCU_VERIFY_TYPE_NONE = 0,
    IDCU_VERIFY_TYPE_CHECKSUM = (1 << 0),
    IDCU_VERIFY_TYPE_SIGNATURE = (1 << 1),
    IDCU_VERIFY_TYPE_MANIFEST = (1 << 2),
    IDCU_VERIFY_TYPE_DEPENDENCY = (1 << 3),
    IDCU_VERIFY_TYPE_SANDBOX = (1 << 4),
    IDCU_VERIFY_TYPE_ALL = 0x1F
} idcu_VerifyType;

typedef enum {
    IDCU_VERIFY_STATUS_UNKNOWN,
    IDCU_VERIFY_STATUS_PENDING,
    IDCU_VERIFY_STATUS_IN_PROGRESS,
    IDCU_VERIFY_STATUS_PASSED,
    IDCU_VERIFY_STATUS_FAILED,
    IDCU_VERIFY_STATUS_SKIPPED
} idcu_VerifyStatus;

typedef struct idcu_ModuleVerifier_Context {
    int initialized;
    idcu_Mutex lock;
    uint64_t operation_count;
    uint64_t error_count;
} idcu_ModuleVerifier_Context;

#ifdef __cplusplus
}
#endif

#endif
