#ifndef IDCU_PERMISSION_PERMISSION_H
#define IDCU_PERMISSION_PERMISSION_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/permission/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_permission_init(idcu_Permission_Context** ctx);
void idcu_permission_destroy(idcu_Permission_Context* ctx);
int idcu_permission_is_initialized(idcu_Permission_Context* ctx);
uint64_t idcu_permission_get_operation_count(idcu_Permission_Context* ctx);
uint64_t idcu_permission_get_error_count(idcu_Permission_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
