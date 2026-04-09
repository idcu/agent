#ifndef IDCU_MANAGEMENT_MANAGEMENT_H
#define IDCU_MANAGEMENT_MANAGEMENT_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/management/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Placeholder API - will be expanded in full implementation
int idcu_management_init(idcu_Management_Context** ctx);
void idcu_management_destroy(idcu_Management_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
