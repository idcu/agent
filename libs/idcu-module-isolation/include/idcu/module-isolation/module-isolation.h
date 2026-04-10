#ifndef IDCU_MODULE-ISOLATION_MODULE-ISOLATION_H
#define IDCU_MODULE-ISOLATION_MODULE-ISOLATION_H

#include <idcu/common/config.h>
#include <idcu/common/error_code.h>
#include <idcu/module-isolation/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int idcu_module-isolation_init(idcu_Module-Isolation_Context** ctx);
void idcu_module-isolation_destroy(idcu_Module-Isolation_Context* ctx);
int idcu_module-isolation_is_initialized(idcu_Module-Isolation_Context* ctx);
uint64_t idcu_module-isolation_get_operation_count(idcu_Module-Isolation_Context* ctx);
uint64_t idcu_module-isolation_get_error_count(idcu_Module-Isolation_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
