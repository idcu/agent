#ifndef IDCU_PERMISSION_PERMISSION_H
#define IDCU_PERMISSION_PERMISSION_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/permission/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_permission_init(idcu_Permission_Context** ctx);
void idcu_permission_destroy(idcu_Permission_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
