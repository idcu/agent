#ifndef IDCU_UTILS_UTILS_H
#define IDCU_UTILS_UTILS_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/utils/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_utils_init(idcu_Utils_Context** ctx);
void idcu_utils_destroy(idcu_Utils_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
