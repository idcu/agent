#ifndef IDCU_MANAGEMENT_MANAGEMENT_H
#define IDCU_MANAGEMENT_MANAGEMENT_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/management/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_management_init(idcu_Management_Context** ctx);
void idcu_management_destroy(idcu_Management_Context* ctx);
int idcu_management_is_initialized(idcu_Management_Context* ctx);
uint64_t idcu_management_get_operation_count(idcu_Management_Context* ctx);
uint64_t idcu_management_get_error_count(idcu_Management_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
